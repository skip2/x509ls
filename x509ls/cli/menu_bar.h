// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_MENU_BAR_H_
#define X509LS_CLI_MENU_BAR_H_

#include <string>

#include "x509ls/base/types.h"
#include "x509ls/cli/base/cli_control.h"
#include "x509ls/cli/base/info_bar.h"

using std::string;

namespace x509ls {
// Menu bar CLI control.
//
// Displays a fixed string of |text|, intended to display keyboard shortcuts
// (e.g. "q:Quit s:Save") on a highlighted row. Intended to be used for the
// first row of a screen layout.
//
// A "Terminal too small!" warning is displayed over the menu when the terminal
// is too small (defined as <15 rows or <50 columns).
class MenuBar : public InfoBar {
 public:
  // Construct a MenuBar with |parent| and fixed menu |text|.
  //
  // |text| should be <=80 characters, to fit on a typical terminal.
  MenuBar(CliControl* parent, const string& text);
  virtual ~MenuBar();

 protected:
  virtual unsigned int LeftTextSize() const;
  virtual void PaintLeftText(int start_col);

 private:
  NO_COPY_AND_ASSIGN(MenuBar)

  const string text_;
};
}  // namespace x509ls

#endif  // X509LS_CLI_MENU_BAR_H_

