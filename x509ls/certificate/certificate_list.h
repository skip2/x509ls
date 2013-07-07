// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CERTIFICATE_CERTIFICATE_LIST_H_
#define X509LS_CERTIFICATE_CERTIFICATE_LIST_H_

#include <openssl/x509.h>

#include <string>
#include <vector>

#include "x509ls/base/types.h"
#include "x509ls/certificate/certificate.h"
#include "x509ls/cli/base/list_model.h"

using std::string;
using std::vector;

namespace x509ls {
// A list of Certificates.
class CertificateList : public ListModel {
 public:
  CertificateList();
  virtual ~CertificateList();

  // Add certificate |x509| with flags |is_in_trust_store|, |is_in_peer_chain|,
  // |is_in_validation_path|. |x509| is cloned. The certificate is added to the
  // end of the list.
  void Add(const X509& x509,
      bool is_in_trust_store = false,
      bool is_in_peer_chain = false,
      bool is_in_validation_path = false);

  // Return the Subject() of the certificate at |index|.
  virtual string Name(size_t index) const;

  // Return the number of certificates.
  virtual size_t Size() const;

  // Return the certificate at |index|.
  const Certificate& operator[](size_t index) const;

 private:
  NO_COPY_AND_ASSIGN(CertificateList)

  vector<Certificate*> list_;
};
}  // namespace x509ls

#endif  // X509LS_CERTIFICATE_CERTIFICATE_LIST_H_

