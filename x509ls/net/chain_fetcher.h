// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_NET_CHAIN_FETCHER_H_
#define X509LS_NET_CHAIN_FETCHER_H_

#include <string>

#include "x509ls/base/base_object.h"
#include "x509ls/base/types.h"
#include "x509ls/net/dns_lookup.h"
#include "x509ls/net/ssl_client.h"

using std::string;

namespace x509ls {
class TrustStore;
// Fetch X509 certificates over TLS asynchronously.
//
// First initiates an DNS lookup on |hostname_or_ip|. If the name is
// successfully resolved (always true for IP address strings), then attempts a
// TLS connection on the specified port. Emits a number of events to indicate
// progress.
class ChainFetcher : public BaseObject {
 public:
  // Construct a ChainFetcher with |parent|, to fetch X509 certificates from
  // |hostname_or_ip| on |port|. Use a DNS lookup of type |lookup_type| and
  // the TLS method (e.g. TLSv1) |tls_method_index|.
  ChainFetcher(BaseObject* parent, TrustStore* trust_store,
      const string& hostname_or_ip, uint16 port,
      DnsLookup::LookupType lookup_type,
      size_t tls_method_index, size_t tls_auth_type_index);

  // Calls Cancel().
  virtual ~ChainFetcher();

  enum State {
    kStateStart,
    kStateResolving,       // Emitted as an event.
    kStateResolveFail,     // Emitted as an event.
    kStateConnecting,      // Emitted as an event.
    kStateConnectSuccess,  // Emitted as an event.
    kStateConnectFail,     // Emitted as an event.
    kStateCancel           // Emitted as an event.
  };
  State GetState() const;

  // Start the fetching process.
  //
  // Call only once.
  //
  // A series of events are Emit()ed during this process:
  // - kStateResolving: DNS lookups started
  // - kStateResolveFail: DNS lookups failed
  // - kStateConnecting: TLS connection started
  // - kStateConnectSuccess: TLS connection succeeded, certificates available
  // - kStateConnectFail: TLS connection failed
  void Start();

  // Cancel any outstanding network requests (DNS/TLS).
  //
  // The state is updated to kStateCancel.
  void Cancel();

  // Receives events from DnsLookup, SslClient objects.
  virtual void OnEvent(const BaseObject* source, int event_code);

  // Methods valid in the kStateConnectSuccess state:
  // Return a string representation of the IP address.
  string IPAddress() const;

  // Return the server's certificate chain.
  const CertificateList* Chain() const;

  // Return the validation path formed by OpenSSL.
  const CertificateList* Path() const;

  // Return the validation status string from OpenSSL.
  string VerifyStatus() const;

  // Methods valid in the kStateResolveFail state:
  // Return the DnsLookup's error message.
  string ErrorMessage() const;

 private:
  NO_COPY_AND_ASSIGN(ChainFetcher)

  TrustStore* const trust_store_;

  const string hostname_or_ip_;
  const uint16 port_;
  const size_t tls_method_index_;
  const size_t tls_auth_type_index_;

  DnsLookup* lookup_;
  SslClient* ssl_client_;

  enum State state_;
  void SetState(const State& state);

  static bool LooksLikeHostname(const string& hostname_or_ip);
};
}  // namespace x509ls

#endif  // X509LS_NET_CHAIN_FETCHER_H_

