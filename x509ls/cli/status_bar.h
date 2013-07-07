// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_STATUS_BAR_H_
#define X509LS_CLI_STATUS_BAR_H_

#include <ncurses.h>

#include <string>

#include "x509ls/base/types.h"
#include "x509ls/cli/base/cli_control.h"
#include "x509ls/cli/base/info_bar.h"

using std::string;

namespace x509ls {
// Simple one-line CLI status bar control.
//
// Contains two text regions: Main (left-aligned) and Extra (right-aligned).
class StatusBar : public InfoBar {
 public:
  StatusBar(CliControl* parent, const string& initial_main_text = "");
  virtual ~StatusBar();

  // Set the displayed text. Repaints as necessary.
  void SetMainText(const string& text);
  void SetExtraText(const string& text);

  // Return the currently displayed text.
  string MainText() const;
  string ExtraText() const;

 protected:
  virtual unsigned int LeftTextSize() const;
  virtual unsigned int RightTextSize() const;

  virtual void PaintLeftText(int start_col);
  virtual void PaintRightText(int start_col);

 private:
  NO_COPY_AND_ASSIGN(StatusBar)

  string main_text_;
  string extra_text_;
};
}  // namespace x509ls

#endif  // X509LS_CLI_STATUS_BAR_H_

