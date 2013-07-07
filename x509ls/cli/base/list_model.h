// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_BASE_LIST_MODEL_H_
#define X509LS_CLI_BASE_LIST_MODEL_H_

#include <string>

using std::string;

namespace x509ls {
// Abstract base class for a simple list.
class ListModel {
 public:
  virtual ~ListModel() {}

  // Return a string representation of the item at |index|.
  virtual string Name(size_t index) const = 0;

  // Return the number of items in the list.
  virtual size_t Size() const = 0;
};
}  // namespace x509ls
#endif  // X509LS_CLI_BASE_LIST_MODEL_H_

