// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/certificate/certificate_list.h"

namespace x509ls {
CertificateList::CertificateList() {
}

// virtual
CertificateList::~CertificateList() {
  for (vector<Certificate*>::iterator it = list_.begin();
      it != list_.end(); ++it) {
    delete *it;
  }
}

void CertificateList::Add(const X509& x509,
    bool is_in_trust_store,
    bool is_in_peer_chain,
    bool is_in_validation_path) {
  list_.push_back(new Certificate(x509,
        is_in_trust_store,
        is_in_peer_chain,
        is_in_validation_path));
}

// virtual
string CertificateList::Name(size_t index) const {
  return list_[index]->Subject();
}

// virtual
size_t CertificateList::Size() const {
  return list_.size();
}

const Certificate& CertificateList::operator[](size_t index) const {
  return *(list_[index]);
}
}  // namespace x509ls

