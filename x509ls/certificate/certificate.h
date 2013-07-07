// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CERTIFICATE_CERTIFICATE_H_
#define X509LS_CERTIFICATE_CERTIFICATE_H_

#include <openssl/x509.h>

#include <string>

using std::string;

#include "x509ls/base/openssl/scoped_openssl.h"
#include "x509ls/base/types.h"

namespace x509ls {
// OpenSSL X509 certificate wrapper.
//
// X509 certificate with some simple accessors. Flags for marking certificates
// as self-signed, in the trust store, peer chain or validation path.
class Certificate {
 public:
  // Construct a Certificate. Clones |x509|.
  //
  // Flags stating if the certificate |is_in_trust_store|, |is_in_peer_chain|,
  // and |is_in_validation_path| are specific to the current trust store and
  // SSL server, and so are determined externally.
  Certificate(const X509& x509,
      bool is_in_trust_store = false,
      bool is_in_peer_chain = false,
      bool is_in_validation_path = false);
  ~Certificate();

  // Return the certificate subject in OpenSSL OneLine format.
  string Subject() const;

  // Return a list of subject common names in CN=x/CN=y/CN=z format. Returns an
  // empty string for certificates with no subject common names.
  string CommonNames() const;

  // Return the certificate NotAfter date in YYYY-MM-DD format, GMT time zone.
  // If the NotAfter date is somehow invalid (e.g. contains letters),
  // "????-??-??" is returned instead.
  string NotAfterDate() const;

  // Return a readable multi-line description of the full certificate,
  // including any X509v3 extensions.
  string TextDescription() const;

  // Return the certificate in PEM format.
  string AsPEM() const;

  // Certificate flags.
  // Determined internally.
  bool IsSelfSigned() const;

  // Set using the constructor.
  bool IsInTrustStore() const;
  bool IsInPeerChain() const;
  bool IsInValidationPath() const;

 private:
  NO_COPY_AND_ASSIGN(Certificate)

  ScopedOpenSSL<X509, void, X509_free> x509_;

  string subject_;
  string common_names_;
  string text_description_;

  bool is_in_trust_store_;
  bool is_in_peer_chain_;
  bool is_in_validation_path_;

  static bool IsNumberString(const unsigned char* start, int length);
};
}  // namespace x509ls

#endif  // X509LS_CERTIFICATE_CERTIFICATE_H_

