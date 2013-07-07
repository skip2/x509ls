// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_BASE_TYPES_H_
#define X509LS_BASE_TYPES_H_

#include <errno.h>

#define NO_COPY_AND_ASSIGN(T) \
  T(const T&); \
  void operator=(const T&);

#define UNUSED(V) \
  (void)V;

typedef unsigned short uint16;  // NOLINT(runtime/int)

// Thanks Chromium :)
#define HANDLE_EINTR(syscall) ({ \
  typeof(syscall) result; \
  do { \
    result = (syscall); \
  } while (result == -1 && errno == EINTR); \
  result; \
})

#endif  // X509LS_BASE_TYPES_H_

