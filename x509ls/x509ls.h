// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_X509LS_H_
#define X509LS_X509LS_H_

#include "x509ls/base/openssl/openssl_environment.h"
#include "x509ls/base/types.h"
#include "x509ls/certificate/trust_store.h"
#include "x509ls/cli/base/cli_application.h"

namespace x509ls {
// Main x509ls application.
//
// Processes command line options. sets up the TrustStore, then starts the
// X509LS ncurses interfaces.
//
// The usage from main() is:
//  X509LS x509ls;
//  if (!x509ls.Init(argc, argv)) {
//    return EXIT_FAILURE;
//  }
//
//  return x509ls.Run() ? EXIT_SUCCESS : EXIT_FAILURE;
class X509LS : public CliApplication {
 public:
  X509LS();
  virtual ~X509LS();

  // Process command line arguments from |argc| and |argv|.
  //
  // Returns true iif the command line arguments could be processed correctly.
  // On failure prints error messages on stdout.
  bool Init(int argc, char** argv);

 protected:
  virtual void RunEvent();

 private:
  NO_COPY_AND_ASSIGN(X509LS)

  ScopedOpenSSLEnvironment openssl_;
  TrustStore trust_store_;
  string host_port_;
};
}  // namespace x509ls

#endif  // X509LS_X509LS_H_

