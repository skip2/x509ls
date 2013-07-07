// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/net/chain_fetcher.h"

#include <assert.h>
#include <ctype.h>

namespace x509ls {
ChainFetcher::ChainFetcher(BaseObject* parent, TrustStore* trust_store,
    const string& hostname_or_ip,
    uint16 port, DnsLookup::AddressFamily address_family,
    size_t tls_method_index, size_t tls_auth_type_index)
  :
  BaseObject(parent),
  trust_store_(trust_store),
  hostname_or_ip_(hostname_or_ip),
  port_(port),
  tls_method_index_(tls_method_index),
  tls_auth_type_index_(tls_auth_type_index),
  lookup_(new DnsLookup(this, hostname_or_ip, port_, address_family)),
  ssl_client_(NULL),
  state_(kStateStart) {
  Subscribe(lookup_, DnsLookup::kStateSuccess);
  Subscribe(lookup_, DnsLookup::kStateFail);
}

// virtual
ChainFetcher::~ChainFetcher() {
  Cancel();
}

ChainFetcher::State ChainFetcher::GetState() const {
  return state_;
}

void ChainFetcher::Start() {
  SetState(kStateResolving);
  lookup_->Start();
}

void ChainFetcher::Cancel() {
  if (state_ == kStateCancel) {
    return;
  }

  Unsubscribe(lookup_);
  if (ssl_client_) {
    Unsubscribe(ssl_client_);
  }

  lookup_->Cancel();

  if (ssl_client_) {
    ssl_client_->Cancel();
  }

  SetState(kStateCancel);
}

// virtual
void ChainFetcher::OnEvent(const BaseObject* source, int event_code) {
  if (source == lookup_ && state_ == kStateResolving) {
    if (event_code == DnsLookup::kStateFail) {
      SetState(kStateResolveFail);
    } else if (event_code == DnsLookup::kStateSuccess) {
      SetState(kStateConnecting);
      assert(ssl_client_ == NULL);

      ssl_client_ = new SslClient(this, trust_store_, lookup_->Sockaddr(),
          lookup_->SockaddrLen(), tls_method_index_, tls_auth_type_index_);

      if (LooksLikeHostname(hostname_or_ip_)) {
        ssl_client_->SetSNIHostname(hostname_or_ip_);
      }

      Subscribe(ssl_client_, SslClient::kStateConnectFail);
      Subscribe(ssl_client_, SslClient::kStateTlsFail);
      Subscribe(ssl_client_, SslClient::kStateSuccess);

      ssl_client_->Connect();
    }
  } else if (source == ssl_client_) {
    switch (event_code) {
    case SslClient::kStateConnectFail:
    case SslClient::kStateTlsFail:
      SetState(kStateConnectFail);
      break;
    case SslClient::kStateSuccess:
      SetState(kStateConnectSuccess);
      break;
    default:
      break;
    }
  }
}

void ChainFetcher::SetState(const State& state) {
  state_ = state;
  Emit(state_);
}

string ChainFetcher::IPAddress() const {
  return lookup_->IPAddress();
}

const CertificateList* ChainFetcher::Chain() const {
  return &(ssl_client_->Chain());
}

const CertificateList* ChainFetcher::Path() const {
  return &(ssl_client_->Path());
}

string ChainFetcher::VerifyStatus() const {
  return ssl_client_->VerifyStatus();
}

// static
bool ChainFetcher::LooksLikeHostname(const string& hostname_or_ip) {
  // Approximate: 0x1.0x2.0x3.0x4 will confuse it.
  for (string::const_iterator it = hostname_or_ip.begin();
      it != hostname_or_ip.end();
      ++it) {
    if (isalpha(*it)) {
      return true;
    }
  }

  return false;
}

string ChainFetcher::ErrorMessage() const {
  if (state_ == kStateResolveFail) {
    return lookup_->ErrorMessage();
  }
  return "";
}
}  // namespace x509ls

