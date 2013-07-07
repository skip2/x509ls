// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/certificate/trust_store.h"

#include <openssl/bio.h>
#include <openssl/err.h>

#include "x509ls/base/openssl/bio_translator.h"

namespace x509ls {
TrustStore::TrustStore()
  :
    store_(X509_STORE_new()) {
}

TrustStore::~TrustStore() {
}

bool TrustStore::AddCAFile(const string& filename, string* error_message) {
  ERR_clear_error();
  *error_message = "";

  X509_LOOKUP* lookup = X509_STORE_add_lookup(store_.Get(), X509_LOOKUP_file());
  if (!lookup) {
    return false;
  }

  if (!X509_LOOKUP_load_file(lookup, filename.c_str(), X509_FILETYPE_PEM)) {
    EmitOpenSSLErrors("Error loading CAFile:", error_message);
    return false;
  }

  return true;
}

bool TrustStore::AddCAPath(const string& directory, string* error_message) {
  ERR_clear_error();
  *error_message = "";

  X509_LOOKUP* lookup = X509_STORE_add_lookup(store_.Get(),
      X509_LOOKUP_hash_dir());
  if (!lookup) {
    return false;
  }

  if (!X509_LOOKUP_add_dir(lookup, directory.c_str(), X509_FILETYPE_PEM)) {
    EmitOpenSSLErrors("Error loading CAPath:", error_message);
    return false;
  }

  return true;
}

bool TrustStore::AddSystemCAPath() {
  return X509_STORE_set_default_paths(store_.Get()) == 1;
}

X509_STORE* TrustStore::Store() {
  return store_.Get();
}

// static
void TrustStore::EmitOpenSSLErrors(const string& message, string* output) {
  BioTranslator errors_bio;
  ERR_print_errors(errors_bio.Get());

  *output = message;
  output->append("\n");
  output->append(errors_bio.ToString());
}
}  // namespace x509ls

