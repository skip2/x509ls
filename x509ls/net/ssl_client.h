// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_NET_SSL_CLIENT_H_
#define X509LS_NET_SSL_CLIENT_H_

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>

#include "x509ls/base/base_object.h"
#include "x509ls/base/openssl/openssl_environment.h"
#include "x509ls/base/types.h"
#include "x509ls/certificate/certificate_list.h"
#include "x509ls/certificate/trust_store.h"

using std::string;

namespace x509ls {
// Connect to an SSL server asynchronously and fetch the certificate chain.
//
//
//
// - SetSNIHostname() allows SNI to be used.
// - Compression (i.e. DEFLATE) is disabled: Not specifically because of the
// security risk in doing so (the CRIME attack...), rather that connections to
// google.com, EC-only currently do not succeed with compression enabled.
class SslClient : public BaseObject {
 public:
  // Construct an SslClient with |parent|, to connect to |saddr| (with length
  // |saddr_len| using TLS method |tls_method_index| and auth type (RSA/EC...)
  // |tls_auth_type_index|. Validate the end-entity certificate found using
  // |trust_store|.
  SslClient(BaseObject* parent, TrustStore* trust_store,
      const sockaddr* saddr, socklen_t saddr_len,
      size_t tls_method_index = 0, size_t tls_auth_type_index = 0);
  virtual ~SslClient();

  // Set the Server Name Indication hostname to |hostname|.
  //
  // Call before Start().
  //
  // Hints to servers serving multiple virtual hosts (and thus potentially
  // serving multiple certificates) which |hostname|, and thus which certificate
  // is required.
  void SetSNIHostname(const string& hostname);

  // Start the network connection.
  //
  // Call only once.
  //
  // The below kState* events will be emitted as the connection progresses.
  void Connect();

  // Cancel the network connection. The state is updated to kStateCancel.
  void Cancel();

  // State machine.
  enum State {
    kStateStart,
    kStateConnecting,   // Emitted as an event.
    kStateConnected,    // Emitted as an event.
    kStateConnectFail,  // Emitted as an event.
    kStateTlsFail,      // Emitted as an event.
    kStateSuccess,      // Emitted as an event.
    kStateCancel        // Emitted as an event.
  };
  // The kStateSuccess state only indicates the server chain could be fetched.
  // It does not refer to certificate validation: SslClient disables certificate
  // validation for connections, to ensure invalid certificates can still be
  // examined.

  // Receives FD readable/writable events.
  virtual void OnFDEvent(int fd, bool read_event,
      bool write_event, bool error_event);

  struct TlsMethod {
    const SSL_METHOD* method;
    const string name;
  };
  static string TlsMethodName(size_t tls_method_index);
  static size_t NextTlsMethod(size_t tls_method_index);

  struct TlsAuthType {
    const string name;
    const string cipherlist;
  };
  static string TlsAuthTypeName(size_t tls_auth_type_index);
  static size_t NextAuthType(size_t tls_auth_type_index);

  // In the kStateSuccess state:
  // Return the server's chain.
  const CertificateList& Chain() const;

  // Return the validation path formed by OpenSSL.
  const CertificateList& Path() const;

  // Return the validation status string from OpenSSL.
  string VerifyStatus() const;

 private:
  NO_COPY_AND_ASSIGN(SslClient)

  ScopedOpenSSLEnvironment openssl_;

  TrustStore* trust_store_;

  static const struct TlsMethod methods_[];
  static const struct TlsAuthType auth_types_[];

  struct sockaddr* saddr_;
  const socklen_t saddr_len_;
  const size_t tls_method_index_;
  const size_t tls_auth_type_index_;

  int fd_;

  State state_;
  void SetState(State state);

  void CloseConnectionWithState(State state);
  void CloseConnection();

  bool SetupOpenSSL();
  void RunOpenSSL(bool can_read, bool can_write);
  void FreeOpenSSL();
  SSL_CTX* ssl_ctx_;
  SSL* ssl_;

  static int VerifyProcedure(int ok, X509_STORE_CTX* ctx);

  CertificateList chain_;
  CertificateList path_;
  string verify_status_;
  void PopulateChainAndPath();

  bool IsX509InTrustStore(const X509* x509) const;
  bool IsX509InPeerChain(const X509* x509) const;

  string sni_name_;
};
}  // namespace x509ls

#endif  // X509LS_NET_SSL_CLIENT_H_

