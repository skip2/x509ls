// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CERTIFICATE_TRUST_STORE_H_
#define X509LS_CERTIFICATE_TRUST_STORE_H_

#include <openssl/x509_vfy.h>

#include <string>

#include "x509ls/base/openssl/openssl_environment.h"
#include "x509ls/base/openssl/scoped_openssl.h"
#include "x509ls/base/types.h"

using std::string;

namespace x509ls {
// Wrapper around the OpenSSL Trust Store.
//
// A trust store contains zero or more trusted certificates, which are used as
// trust anchors in the final step of certificate validation.
//
// Provides methods to add files containing trusted certificates, add OpenSSL
// style directories containing trusted certificates, and add the default system
// trust store.
class TrustStore {
 public:
  TrustStore();
  ~TrustStore();

  // The following methods return true iif OpenSSL accepted the trusted
  // certificate(s) being added. All certificates should be in PEM format. On
  // error, |error_message| is set to a short string describing the problem.

  // Trust the certificate(s) in |filename|.
  bool AddCAFile(const string& filename, string* error_message);

  // Trust the certificates in the OpenSSL trusted certificate directory
  // |directory|.
  bool AddCAPath(const string& directory, string* error_message);
  bool AddSystemCAPath();

  // Return the underlying trust store.
  X509_STORE* Store();

 private:
  NO_COPY_AND_ASSIGN(TrustStore)

  ScopedOpenSSLEnvironment openssl_;
  ScopedOpenSSL<X509_STORE, void, X509_STORE_free> store_;

  static void EmitOpenSSLErrors(const string& message, string* output);
};
}  // namespace x509ls

#endif  // X509LS_CERTIFICATE_TRUST_STORE_H_

