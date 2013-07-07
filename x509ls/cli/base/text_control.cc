// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/cli/base/text_control.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include <algorithm>

using std::min;
using std::max;

namespace x509ls {
TextControl::TextControl(CliControl* parent, const string& text)
  :
    CliControl(parent),
    pad_(NULL),
    text_(text),
    first_visible_line_index_(0),
    line_count_(0) {
}

TextControl::~TextControl() {
  if (pad_) {
    delwin(pad_);
  }
}

bool TextControl::Scroll(const enum ScrollDirection direction, int lines) {
  if (!pad_) {
    return false;
  }

  WINDOW* window = Window();
  assert(window != NULL);

  int adjustment = lines != 0 ? lines : Rows();
  if (direction == kDirectionUp) {
    adjustment = -adjustment;
  }

  int candidate_index = first_visible_line_index_ + adjustment;

  candidate_index = max(candidate_index, 0);
  candidate_index = min(static_cast<unsigned int>(candidate_index),
      line_count_ - Rows());
  candidate_index = max(candidate_index, 0);

  if (static_cast<unsigned int>(candidate_index) == first_visible_line_index_) {
    return false;
  }

  first_visible_line_index_ = candidate_index;

  return true;
}

// virtual
void TextControl::PaintEvent() {
  WINDOW* window = Window();
  if (!window) {
    return;
  }

  bool render_pad = false;

  if (!pad_) {
    render_pad = true;
  } else {
    int pad_rows, pad_cols;

    UNUSED(pad_rows);

    getmaxyx(pad_, pad_rows, pad_cols);

    if (pad_cols != Cols()) {
      render_pad = true;
    }
  }

  if (render_pad) {
    unsigned int first_byte_offset = 0;
    map<unsigned int, unsigned int>::const_iterator byte_it =
      line_to_byte_offset_.find(first_visible_line_index_);
    if (byte_it != line_to_byte_offset_.end()) {
      first_byte_offset = byte_it->second;
    }

    line_to_byte_offset_.clear();

    if (pad_) {
      delwin(pad_);
      pad_ = NULL;
    }

    pad_ = newpad(1000, Cols());

    line_count_ = 0;
    for (string::const_iterator it = text_.begin();
        it != text_.end();
        ++it) {
      int row, col;
      getyx(pad_, row, col);

      if (col == 0) {
        line_count_++;

        line_to_byte_offset_[row] = it - text_.begin();

        if (static_cast<unsigned int>(it - text_.begin()) <=
            first_byte_offset) {
          first_visible_line_index_ = row;
        }
      }
      waddch(pad_, *it);
    }
  }

  wclear(window);

  copywin(pad_, window, first_visible_line_index_, 0,
      0, 0, Rows() - 1, Cols() - 1, false);

  wnoutrefresh(window);
}

// virtual
bool TextControl::KeyPressEvent(int keypress) {
  bool handled = false;

  switch (keypress) {
  case KEY_DOWN:
    Scroll(kDirectionDown, 1);
    handled = true;
    break;
  case KEY_UP:
    Scroll(kDirectionUp, 1);
    handled = true;
    break;
  case KEY_NPAGE:
  case ' ':
    Scroll(kDirectionDown);
    handled = true;
    break;
  case KEY_PPAGE:
    Scroll(kDirectionUp);
    handled = true;
    break;
  }

  if (handled) {
    Repaint();
  }

  return handled;
}

void TextControl::SetText(const string& text) {
  text_ = text;
  first_visible_line_index_ = 0;
  line_count_ = 0;
  line_to_byte_offset_.clear();

  if (pad_) {
    delwin(pad_);
    pad_ = NULL;
  }

  Repaint();
}

}  // namespace x509ls

