// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_BASE_OPENSSL_BIO_TRANSLATOR_H_
#define X509LS_BASE_OPENSSL_BIO_TRANSLATOR_H_

#include <openssl/bio.h>

#include <string>

#include "x509ls/base/openssl/scoped_openssl.h"

using std::string;

namespace x509ls {
// An OpenSSL memory BIO sink, with a std::string accessor.
//
// Enables the output of OpenSSL functions which write to a BIO (an IO
// abstraction) to be accessed via a std::string.
//
// Provides a BIO to write into, and a method to read the contents as a
// std::string.
class BioTranslator {
 public:
  BioTranslator();

  BIO* Get() const;
  string ToString() const;

 private:
  NO_COPY_AND_ASSIGN(BioTranslator)

  ScopedOpenSSL<BIO, int, BIO_free> bio_;
};
}  // namespace x509ls

#endif  // X509LS_BASE_OPENSSL_BIO_TRANSLATOR_H_

