// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/cli/base/info_bar.h"

#include <ncurses.h>
#include <string.h>

#include <algorithm>

#include "x509ls/cli/base/colours.h"

using std::max;

namespace x509ls {
InfoBar::InfoBar(CliControl* parent)
  :
    CliControl(parent) {
}

// virtual
InfoBar::~InfoBar() {
}

// virtual
int InfoBar::PreferredHeight() const {
  return 1;
}

// virtual
void InfoBar::PaintEvent() {
  WINDOW* window = Window();

  wclear(window);
  wbkgd(window, Colours::Get(Colours::kColourInfoBar));

  PaintLeftText(0);

  const int left_text_size = LeftTextSize();
  const int minimum_right_start_column = left_text_size + 2;
  const int maximum_right_start_column = Cols() - RightTextSize();
  const int remaining_columns = Cols() - left_text_size - 2;

  if (remaining_columns > 0) {
    PaintRightText(max(
          maximum_right_start_column,
          minimum_right_start_column));
  }

  wnoutrefresh(window);
}

// virtual
unsigned int InfoBar::RightTextSize() const {
  return 0;
}

// virtual
void InfoBar::PaintRightText(int start_col) {
}
}  // namespace x509ls

