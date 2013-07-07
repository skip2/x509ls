// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/cli/base/cli_control.h"

#include <stdio.h>

#include <algorithm>
#include <iterator>

#include "x509ls/cli/base/cli_application.h"

using std::min;

namespace x509ls {
CliControl::CliControl(CliControl* parent)
  :
    BaseObject(parent),
    parent_(parent),
    focused_child_(NULL),
    has_focus_(false),
    my_window_(NULL),
    show_cursor_(false),
    cursor_y(0),
    cursor_x(0),
    rows_(0),
    cols_(0) {
}

CliControl::CliControl(CliApplication* application)
  :
    BaseObject(application),
    parent_(NULL),
    focused_child_(NULL),
    has_focus_(false),
    my_window_(NULL),
    show_cursor_(false),
    cursor_y(0),
    cursor_x(0),
    rows_(0),
    cols_(0) {
}

CliControl::~CliControl() {
  for (list<ChildControl>::iterator it = children_.begin();
      it != children_.end();
      ++it) {
    if (it->window) {
      it->control->SetWindow(NULL);
      delwin(it->window);
    }
  }

  children_.clear();
}

bool CliControl::OnKeyPress(int keypress) {
  bool handled = false;

  if (focused_child_ && focused_child_->OnKeyPress(keypress)) {
    handled = true;
  } else if (KeyPressEvent(keypress)) {
    handled = true;
  }

  return handled;
}

void CliControl::Repaint(bool recursive_repaint) {
  if (!my_window_) {
    return;
  }

  wclear(my_window_);

  for (list<ChildControl>::iterator it = children_.begin();
      it != children_.end();
      ++it) {
    it->control->Repaint(true);
  }

  PaintEvent();

  wnoutrefresh(my_window_);

  if (!recursive_repaint) {
    // Top level Repaint() call only restores focus.
    GetApplication()->FocusedControl()->OnFocus();
  }
}

// virtual
bool CliControl::KeyPressEvent(int keypress) {
  return false;
}

// virtual
void CliControl::PaintEvent() {
}

// virtual
void CliControl::ResizeEvent() {
}

void CliControl::SetWindow(WINDOW* window) {
  bool call_resize_event = false;
  int original_rows = -1;
  int original_cols = -1;

  if (my_window_ != NULL) {
    getmaxyx(my_window_, original_rows, original_cols);
  } else {
    rows_ = 0;
    cols_ = 0;
  }

  my_window_ = window;

  if (window == NULL) {
    for (list<ChildControl>::iterator it = children_.begin();
        it != children_.end();
        ++it) {
      if (it->window != NULL) {
        it->control->SetWindow(NULL);

        delwin(it->window);
        it->window = NULL;
      }
    }

    rows_ = 0;
    cols_ = 0;

    return;
  }

  getmaxyx(my_window_, rows_, cols_);

  if (Rows() != original_rows || Cols() != original_cols) {
    call_resize_event = true;
  }

  if (children_.size() > 0) {
    int required_rows = 0;
    int number_of_expanding_controls = 0;

    for (list<ChildControl>::const_iterator it = children_.begin();
       it != children_.end();
       ++it) {
      if (it->required_rows == -1) {
        number_of_expanding_controls++;
      } else {
        required_rows += it->required_rows;
      }
    }

    int current_row = 0;
    for (list<ChildControl>::iterator it = children_.begin();
       it != children_.end();
       ++it) {
      if (Rows() - current_row == 0 ||
          (it->required_rows != -1 &&
           Rows() - current_row < it->required_rows)) {
        // Ran out of space, don't assign a subwindow for this, it can't render.
        it->control->SetWindow(NULL);

        if (it->window) {
          delwin(it->window);
          it->window = NULL;
        }

        current_row  = Rows();
      } else {
        int rows_to_assign = 0;
        if (it->required_rows == -1) {
          rows_to_assign = min(Rows() - current_row,
              (Rows() - required_rows) / number_of_expanding_controls);
        } else {
          rows_to_assign = it->required_rows;
        }

        WINDOW* new_window = subwin(my_window_, rows_to_assign, Cols(),
            current_row, 0);

        it->control->SetWindow(new_window);

        if (it->window) {
          delwin(it->window);
        }

        it->window = new_window;

        current_row += rows_to_assign;
      }
    }
  }

  if (call_resize_event) {
    ResizeEvent();
  }
}

WINDOW* CliControl::Window() const {
  return my_window_;
}

// virtual
int CliControl::PreferredHeight() const {
  return -1;
}

void CliControl::AddChild(CliControl* control) {
  const struct ChildControl child(control, NULL, control->PreferredHeight());
  children_.push_back(child);

  SetWindow(Window());

  Repaint();
}

void CliControl::ReplaceChild(int index, CliControl* control) {
  list<ChildControl>::iterator it = children_.begin();
  std::advance(it, index);

  it->control->SetWindow(NULL);

  ChildControl new_control(control, it->window, control->PreferredHeight());

  *it = new_control;

  SetWindow(Window());

  Repaint();
}

void CliControl::SetFocusedChild(CliControl* control) {
  if (control == focused_child_) {
    return;
  }

  if (focused_child_ == NULL) {
    OnBlur();
  } else {
    focused_child_->OnBlur();
  }

  if (control == NULL) {
    OnFocus();
    focused_child_ = NULL;
  } else {
    focused_child_ = control;
    focused_child_->OnFocus();
  }
}

void CliControl::OnFocus() {
  has_focus_ = true;
  UpdateCursor();
}

void CliControl::OnBlur() {
  has_focus_ = false;

  curs_set(0);
  wnoutrefresh(Window());
}

void CliControl::SetCursor(unsigned int y, unsigned int x) {
  cursor_y = y;
  cursor_x = x;
  show_cursor_ = true;
  UpdateCursor();
}

void CliControl::HideCursor() {
  show_cursor_ = false;
  UpdateCursor();
}

void CliControl::UpdateCursor() {
  if (has_focus_) {
    wmove(Window(), cursor_y, cursor_x);

    curs_set(show_cursor_ ? 1 : 0);
    wnoutrefresh(Window());
  }
}

CliControl* CliControl::FocusedChild() {
  return focused_child_;
}

int CliControl::Rows() const {
  return rows_;
}

int CliControl::Cols() const {
  return cols_;
}
}  // namespace x509ls

