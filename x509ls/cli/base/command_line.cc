// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/cli/base/command_line.h"

#include <ctype.h>
#include <ncurses.h>

#include <algorithm>

#include "x509ls/cli/base/colours.h"

using std::max;
using std::min;

namespace x509ls {
CommandLine::CommandLine(CliControl* parent)
  :
    CliControl(parent),
    cursor_offset_(0),
    is_accepting_input_text_(false) {
}

// virtual
CommandLine::~CommandLine() {
}

// virtual
int CommandLine::PreferredHeight() const {
  return 1;
}

// virtual
bool CommandLine::KeyPressEvent(int keypress) {
  // TODO(tfh): wide character support.
  // In particular needs careful attention to measuring the size of multi-byte
  // characters, to handle KEY_BACKSPACE and other events.
  if (!is_accepting_input_text_) {
    return false;
  }

  if (!Window()) {
    return false;
  }

  WINDOW* window = Window();

  curs_set(1);

  bool force_repaint = false;
  if (isprint(keypress) && isascii(keypress)) {
    if (cursor_offset_ == input_text_.size() + 1) {
      cursor_offset_++;
      input_text_.push_back(static_cast<const char>(keypress));
      waddch(window, keypress);
    } else {
      input_text_.insert(cursor_offset_, 1, static_cast<const char>(keypress));
      cursor_offset_++;
      force_repaint = true;
    }
  } else if (keypress == KEY_BACKSPACE) {
    if (cursor_offset_ > 0) {
      input_text_.erase(cursor_offset_ - 1, 1);
      cursor_offset_--;
      force_repaint = true;
    } else {
      Emit(kEventInputCancelled);
    }
  } else if (keypress == KEY_DC) {
    if (cursor_offset_ < input_text_.size()) {
      input_text_.erase(cursor_offset_, 1);
      force_repaint = true;
    }
    // Delete key.
  } else if (keypress == 0x0A) {
    // Accept.
    Emit(kEventInputAccepted);
  } else if (keypress == 0x1B) {
    // Escape key.
    Emit(kEventInputCancelled);
  } else if (keypress == KEY_LEFT) {
    cursor_offset_ = max(0, static_cast<const int>(cursor_offset_ - 1));
    PositionCursor();
  } else if (keypress == KEY_RIGHT) {
    cursor_offset_ = min(static_cast<const unsigned int>(input_text_.size()),
        cursor_offset_ + 1);
    PositionCursor();
  } else if (keypress == KEY_HOME) {
    cursor_offset_ = 0;
    PositionCursor();
  } else if (keypress == KEY_END) {
    cursor_offset_ = input_text_.size() == 0 ? 0 : input_text_.size();
    PositionCursor();
  }

  if (force_repaint) {
    Repaint();
  } else {
    wnoutrefresh(window);
  }

  return true;
}

// virtual
void CommandLine::PaintEvent() {
  WINDOW* window = Window();

  wclear(window);

  wmove(window, 0, 0);
  wprintw(window, prompt_text_.c_str());
  wprintw(window, input_text_.c_str());
  PositionCursor();

  wnoutrefresh(window);
}

void CommandLine::PositionCursor() {
  SetCursor(0, prompt_text_.size() + cursor_offset_);
}

string CommandLine::InputText() const {
  return input_text_;
}

void CommandLine::DisplayMessage(const string& message_text) {
  prompt_text_ = message_text;
  input_text_ = "";
  cursor_offset_ = 0;
  is_accepting_input_text_ = false;
  HideCursor();
  Repaint();
}

void CommandLine::DisplayPrompt(const string& prompt_question,
    const string& input_text) {
  prompt_text_ = prompt_question;
  input_text_ = input_text;
  cursor_offset_ = 0;
  is_accepting_input_text_ = true;
  PositionCursor();
  Repaint();
}

void CommandLine::Clear() {
  prompt_text_ = "";
  input_text_ = "";
  cursor_offset_ = 0;
  is_accepting_input_text_ = false;
  HideCursor();
  Repaint();
}
}  // namespace x509ls

