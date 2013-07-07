// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/cli/menu_bar.h"

#include <ncurses.h>

#include "x509ls/cli/base/colours.h"

namespace x509ls {
MenuBar::MenuBar(CliControl* parent, const string& text)
  :
    InfoBar(parent),
    text_(text) {
}

// virtual
MenuBar::~MenuBar() {
}

// virtual
unsigned int MenuBar::LeftTextSize() const {
  return text_.size();
}

// virtual
void MenuBar::PaintLeftText(int start_col) {
  WINDOW* window = Window();

  wmove(window, 0, start_col);
  wattron(window, Colours::Get(Colours::kColourInfoBar));
  wprintw(window, text_.c_str());

  // Print a warning if the terminal is too small.
  int rows, cols;
  getmaxyx(stdscr, rows, cols);
  UNUSED(cols);
  if (rows < 15 || cols < 50) {  // Picked by eye.
    wmove(window, 0, start_col);
    wattron(window, Colours::Get(Colours::kColourWarning));
    wprintw(window, "Terminal too small!");
  }
}
}  // namespace x509ls

