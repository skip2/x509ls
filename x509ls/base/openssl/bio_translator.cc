// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/base/openssl/bio_translator.h"

namespace x509ls {
BioTranslator::BioTranslator()
  :
    bio_(BIO_new(BIO_s_mem())) {
}

BIO* BioTranslator::Get() const {
  return bio_.Get();
}

string BioTranslator::ToString() const {
  char* data;
  long length = BIO_get_mem_data(bio_.Get(), &data);  // NOLINT(runtime/int)

  return string(data, length);
}
}  // namespace x509ls

