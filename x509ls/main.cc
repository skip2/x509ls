// X509LS
// Copyright 2013 Tom Harwood

#include <stdlib.h>

#include "x509ls/x509ls.h"

using x509ls::X509LS;

int main(int argc, char** argv) {
  X509LS x509ls;
  if (!x509ls.Init(argc, argv)) {
    return EXIT_FAILURE;
  }

  return x509ls.Run() ? EXIT_SUCCESS : EXIT_FAILURE;
}

