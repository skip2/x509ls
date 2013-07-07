// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_BASE_OPENSSL_SCOPED_OPENSSL_H_
#define X509LS_BASE_OPENSSL_SCOPED_OPENSSL_H_

#include "x509ls/base/types.h"

namespace x509ls {
// Neat idea from Chromium for automatically cleaning up OpenSSL objects.  Like
// a boost::scoped_ptr (RAII), but additionally provides a method to be called
// to destroy the underlying pointer.
//
// example usage:
// ScopedOpenSSL<BIO, int, BIO_free> bio(BIO_new(BIO_s_mem()));
template <typename T, typename R, R (*Destructor)(T* t)>
class ScopedOpenSSL {
 public:
  explicit ScopedOpenSSL(T* ptr)
    :
      ptr_(ptr) {
  }

  ~ScopedOpenSSL() {
    Destructor(ptr_);
  }

  T* Get() const {
    return ptr_;
  }

 private:
  NO_COPY_AND_ASSIGN(ScopedOpenSSL)

  T* const ptr_;
};
}  // namespace x509ls

#endif  // X509LS_BASE_OPENSSL_SCOPED_OPENSSL_H_

