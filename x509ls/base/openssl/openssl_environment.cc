// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/base/openssl/openssl_environment.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>

namespace x509ls {
int ScopedOpenSSLEnvironment::recurse_level_ = 0;

ScopedOpenSSLEnvironment::ScopedOpenSSLEnvironment() {
  recurse_level_++;

  if (recurse_level_ == 1) {
    SSL_load_error_strings();
    SSL_library_init();
  }
}

ScopedOpenSSLEnvironment::~ScopedOpenSSLEnvironment() {
  recurse_level_--;

  if (recurse_level_ == 0) {
    EVP_cleanup();
    ERR_free_strings();
  }
}
}  // namespace x509ls

