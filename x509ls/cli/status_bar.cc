// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/cli/status_bar.h"

#include "x509ls/cli/base/colours.h"

namespace x509ls {
StatusBar::StatusBar(CliControl* parent, const string& inital_main_text)
  :
    InfoBar(parent),
    main_text_(inital_main_text) {
}

// virtual
StatusBar::~StatusBar() {
}

void StatusBar::SetMainText(const string& text) {
  main_text_ = text;
  Repaint();
}

string StatusBar::MainText() const {
  return main_text_;
}

void StatusBar::SetExtraText(const string& text) {
  extra_text_ = text;
  Repaint();
}

string StatusBar::ExtraText() const {
  return extra_text_;
}

// virtual
unsigned int StatusBar::LeftTextSize() const {
  return main_text_.size();
}

// virtual
unsigned int StatusBar::RightTextSize() const {
  return extra_text_.size();
}

// virtual
void StatusBar::PaintLeftText(int start_col) {
  WINDOW* window = Window();

  wmove(window, 0, start_col);
  wattron(window, Colours::Get(Colours::kColourInfoBar));
  wprintw(window, "%s", main_text_.c_str());
}

// virtual
void StatusBar::PaintRightText(int start_col) {
  WINDOW* window = Window();

  wmove(window, 0, start_col);
  wattron(window, Colours::Get(Colours::kColourInfoBar));
  wprintw(window, "%s", extra_text_.c_str());
}
}  // namespace x509ls

