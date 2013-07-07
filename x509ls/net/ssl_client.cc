// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/net/ssl_client.h"

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <openssl/safestack.h>
#include <openssl/stack.h>
#include <openssl/x509.h>

#include <list>

#include "x509ls/base/event_manager.h"

using std::list;

namespace x509ls {
// static
const struct SslClient::TlsMethod
  SslClient::methods_[] = {
  {TLSv1_client_method(), "TLSv1"},
#ifdef SSL_TXT_TLSV1_1
  {TLSv1_1_client_method(), "TLSv1.1"},
#endif
#ifdef SSL_TXT_TLSV1_2
  {TLSv1_2_client_method(), "TLSv1.2"},
#endif
  {SSLv3_client_method(), "SSLv3"},
  {SSLv23_client_method(), "SSLv2,3+TLSv1"},
  {NULL, ""}
};

// static
const struct SslClient::TlsAuthType SslClient::auth_types_[] = {
#if defined(SSL_TXT_ECDSA) && !defined(OPENSSL_NO_ECDSA)
  {"RSA|EC|DSS", SSL_TXT_aRSA ":" SSL_TXT_ECDSA ":" SSL_TXT_aDSS},
  {"EC-only", SSL_TXT_ECDSA},
#else
  {"RSA|DSS", SSL_TXT_aRSA ":" SSL_TXT_aDSS},
#endif
  {"RSA-only", SSL_TXT_aRSA},
  {"DSS-only", SSL_TXT_aDSS},
  {"all", SSL_TXT_ALL},
  {"", ""}
};

SslClient::SslClient(BaseObject* parent, TrustStore* trust_store,
    const sockaddr* saddr, socklen_t saddr_len,
    size_t tls_method_index, size_t tls_auth_type_index)
  :
    BaseObject(parent),
    trust_store_(trust_store),
    saddr_len_(saddr_len),
    tls_method_index_(tls_method_index),
    tls_auth_type_index_(tls_auth_type_index),
    fd_(-1),
    state_(kStateStart),
    ssl_ctx_(NULL),
    ssl_(NULL),
    chain_(),
    path_() {
  saddr_ = static_cast<sockaddr*>(malloc(saddr_len));
  memcpy(saddr_, saddr, saddr_len_);
}

// virtual
SslClient::~SslClient() {
  CloseConnection();

  free(saddr_);

  if (ssl_) {
    SSL_free(ssl_);
  }

  if (ssl_ctx_) {
    SSL_CTX_free(ssl_ctx_);
  }
}

void SslClient::Connect() {
  assert(fd_ == -1);

  SetState(kStateConnecting);

  fd_ = socket(saddr_->sa_family, SOCK_STREAM, 0);
  fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL) | O_NONBLOCK);

  WatchFD(fd_, EventManager::kFDWritable);

  int result = connect(fd_, saddr_, saddr_len_);
  if (result == -1 && errno != EINPROGRESS) {
    SetState(kStateConnectFail);
    UnwatchFD(fd_);
  } else if (result == 0) {
    SetState(kStateConnected);
    if (!SetupOpenSSL()) {
      CloseConnectionWithState(kStateTlsFail);
      return;
    }
    RunOpenSSL(false, true);
  }

  // Connection in progress: Wait for FD events delivered to OnFDEvent().
}

void SslClient::Cancel() {
  CloseConnectionWithState(kStateCancel);
}

// virtual
void SslClient::OnFDEvent(int fd, bool read_event,
    bool write_event, bool error_event) {
  if (state_ == kStateConnecting && write_event) {
    char result;
    socklen_t result_len = sizeof result;
    int success = getsockopt(fd_, SOL_SOCKET, SO_ERROR, &result, &result_len);

    if (success != 0 || result_len != 1 || result != 0) {
      CloseConnectionWithState(kStateConnectFail);
    } else {
      SetState(kStateConnected);
      if (!SetupOpenSSL()) {
        CloseConnectionWithState(kStateTlsFail);
      }
    }
  }

  if (state_ != kStateConnected) {
    return;
  }

  RunOpenSSL(read_event, write_event);
}

bool SslClient::SetupOpenSSL() {
  ssl_ctx_ = SSL_CTX_new(
      const_cast<SSL_METHOD*>(methods_[tls_method_index_].method));
  if (ssl_ctx_ == NULL) {
    return false;
  }

  SSL_CTX_set_cipher_list(ssl_ctx_,
      auth_types_[tls_auth_type_index_].cipherlist.c_str());

  SSL_CTX_set_options(ssl_ctx_, SSL_OP_NO_COMPRESSION);

  SSL_CTX_set_verify(ssl_ctx_, SSL_VERIFY_NONE, VerifyProcedure);

  ssl_ = SSL_new(ssl_ctx_);
  if (!ssl_) {
    goto err1;
  }

  if (SSL_set_fd(ssl_, fd_) != 1) {
    goto err2;
  }

#ifdef SSL_set_tlsext_host_name
  SSL_set_tlsext_host_name(ssl_, sni_name_.c_str());
#endif

  return true;

 err2:
  SSL_free(ssl_);
  ssl_ = NULL;

 err1:
  SSL_CTX_free(ssl_ctx_);
  ssl_ctx_ = NULL;

  return false;
}

void SslClient::RunOpenSSL(bool can_read, bool can_write) {
  int result = SSL_connect(ssl_);
  if (result == 1) {
    PopulateChainAndPath();
    CloseConnectionWithState(kStateSuccess);
    return;
  }

  bool fatal_error = false;

  if (result == 0) {
    fatal_error = true;
  } else if (result == -1) {
    int error = SSL_get_error(ssl_, result);

    if (error == SSL_ERROR_WANT_READ) {
      WatchFD(fd_, EventManager::kFDReadable);
    } else if (error == SSL_ERROR_WANT_WRITE) {
      WatchFD(fd_, EventManager::kFDWritable);
    } else {
      fatal_error = true;
    }
  }

  if (fatal_error) {
    CloseConnectionWithState(kStateTlsFail);
  }
}

// static
string SslClient::TlsMethodName(size_t tls_method_index) {
  return methods_[tls_method_index].name;
}

// static
size_t SslClient::NextTlsMethod(size_t tls_method_index) {
  tls_method_index++;
  if (methods_[tls_method_index].method == NULL) {
    tls_method_index = 0;
  }
  return tls_method_index;
}

// static
string SslClient::TlsAuthTypeName(size_t tls_auth_type_index) {
  return auth_types_[tls_auth_type_index].name;
}

// static
size_t SslClient::NextAuthType(size_t tls_auth_type_index) {
  tls_auth_type_index++;
  if (auth_types_[tls_auth_type_index].cipherlist == "") {
    tls_auth_type_index = 0;
  }
  return tls_auth_type_index;
}

void SslClient::SetState(State state) {
  state_ = state;
  Emit(state_);
}

void SslClient::CloseConnectionWithState(State state) {
  CloseConnection();
  SetState(state);
}

void SslClient::CloseConnection() {
  assert(fd_ != -1);

  UnwatchFD(fd_);
  close(fd_);
}

// static
int SslClient::VerifyProcedure(int ok, X509_STORE_CTX* ctx) {
  return ok;
}

void SslClient::PopulateChainAndPath() {
  STACK_OF(X509)* peer_chain = SSL_get_peer_cert_chain(ssl_);
  if (peer_chain == NULL || sk_X509_num(peer_chain) < 1) {
    return;
  }

  list<X509*> verification_path;

  // Reverify the peer chain to gain access to the verification chain.
  X509* cert = sk_X509_value(peer_chain, 0);

  X509_STORE_CTX ctx;
  X509_STORE_CTX_init(&ctx, trust_store_->Store(), cert, peer_chain);

  int result = X509_verify_cert(&ctx);
  if (result >= 0) {
    for (int i = sk_X509_num(ctx.chain) - 1; i >= 0; --i) {
      X509* x509 = sk_X509_value(ctx.chain, i);
      verification_path.push_back(x509);

      path_.Add(*x509,
          IsX509InTrustStore(x509),
          IsX509InPeerChain(x509),
          true);
    }
  }

  // Populate peer chain.
  for (int i = 0; i < sk_X509_num(peer_chain); ++i) {
    X509* x509 = sk_X509_value(peer_chain, i);

    bool in_verification_path = false;
    for (list<X509*>::const_iterator it = verification_path.begin();
        it != verification_path.end();
        ++it) {
      if (X509_cmp(x509, *it) == 0) {
        in_verification_path = true;
        break;
      }
    }

    chain_.Add(*x509,
        IsX509InTrustStore(x509),
        true,
        in_verification_path);
  }

  verify_status_ = X509_verify_cert_error_string(
      X509_STORE_CTX_get_error(&ctx));

  X509_STORE_CTX_cleanup(&ctx);
}

const CertificateList& SslClient::Chain() const {
  return chain_;
}

const CertificateList& SslClient::Path() const {
  return path_;
}

#ifdef X509LS_OLD_OPENSSL_NO_TRUST_STORE_LOOKUP
bool SslClient::IsX509InTrustStore(const X509* x509) const {
  return false;
}
#else
bool SslClient::IsX509InTrustStore(const X509* x509) const {
  X509_STORE_CTX ctx;
  X509_STORE_CTX_init(&ctx, trust_store_->Store(), NULL, NULL);

  STACK_OF(X509)* candidates = X509_STORE_get1_certs(&ctx,
      X509_get_subject_name(const_cast<X509*>(x509)));

  bool is_in_trust_store = false;

  if (candidates != NULL) {
    for (int i = 0; i < sk_X509_num(candidates); ++i) {
      if (X509_cmp(x509, sk_X509_value(candidates, i)) == 0) {
        is_in_trust_store = true;
        break;
      }
    }
  }

  X509_STORE_CTX_cleanup(&ctx);

  return is_in_trust_store;
}
#endif

bool SslClient::IsX509InPeerChain(const X509* x509) const {
  STACK_OF(X509)* peer_chain = SSL_get_peer_cert_chain(ssl_);

  if (peer_chain != NULL) {
    for (int i = 0; i < sk_X509_num(peer_chain); ++i) {
      if (X509_cmp(x509, sk_X509_value(peer_chain, i)) == 0) {
        return true;
      }
    }
  }

  return false;
}

void SslClient::SetSNIHostname(const string& hostname) {
  sni_name_ = hostname;
}

string SslClient::VerifyStatus() const {
  return verify_status_;
}
}  // namespace x509ls

