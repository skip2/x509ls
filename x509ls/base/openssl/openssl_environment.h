// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_BASE_OPENSSL_OPENSSL_ENVIRONMENT_H_
#define X509LS_BASE_OPENSSL_OPENSSL_ENVIRONMENT_H_

#include <openssl/opensslv.h>

#include "x509ls/base/types.h"

#if OPENSSL_VERSION_NUMBER < 0x1000000fL
#define X509LS_OLD_OPENSSL_NO_TRUST_STORE_LOOKUP
#endif

namespace x509ls {
// Ensures OpenSSL is initialised while in scope.
//
// OpenSSL has several initialisation & cleanup functions. Using internal
// reference counting, OpenSSL is kept initialised while one or more
// ScopedOpenSSLEnvironment objects are in scope. Runs the necessary OpenSSL
// cleanup functions when the last ScopedOpenSSLEnvironment is destroyed.
class ScopedOpenSSLEnvironment {
 public:
  ScopedOpenSSLEnvironment();
  ~ScopedOpenSSLEnvironment();

 private:
  NO_COPY_AND_ASSIGN(ScopedOpenSSLEnvironment)

  static int recurse_level_;
};
}  // namespace x509ls

#endif  // X509LS_BASE_OPENSSL_OPENSSL_ENVIRONMENT_H_

