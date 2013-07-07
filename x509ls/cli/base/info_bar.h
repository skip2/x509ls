// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_BASE_INFO_BAR_H_
#define X509LS_CLI_BASE_INFO_BAR_H_

#include <string>

#include "x509ls/base/types.h"
#include "x509ls/cli/base/cli_control.h"

using std::string;

namespace x509ls {
// A single line, highlighted information bar. Abstract base class.
//
// Displays two strings in a single coloured terminal line. The two strings are
// left-aligned and right-aligned respectively.
//
// Painting the actual text is delegated to subclasses.
class InfoBar : public CliControl {
 public:
  explicit InfoBar(CliControl* parent);
  virtual ~InfoBar();

  // Return the preferred render height of InfoBar, 1.
  virtual int PreferredHeight() const;

 protected:
  virtual void PaintEvent();

  // Methods returning the number of characters in each string.
  virtual unsigned int LeftTextSize() const = 0;
  virtual unsigned int RightTextSize() const;

  // Paint each string. |start_col| is the column text should start at.
  virtual void PaintLeftText(int start_col) = 0;
  virtual void PaintRightText(int start_col);

 private:
  NO_COPY_AND_ASSIGN(InfoBar)
};
}  // namespace x509ls

#endif  // X509LS_CLI_BASE_INFO_BAR_H_

