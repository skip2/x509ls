// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/cli/base/list_control.h"

#include <ncurses.h>

#include <algorithm>

#include "x509ls/cli/base/colours.h"

using std::min;

namespace x509ls {
// static
const int ListControl::kEventSelectedItemChanged = 1;

ListControl::ListControl(CliControl* parent, const ListModel* model)
  :
    CliControl(parent),
    model_(model),
    selected_index_(0) {
}

// virtual
ListControl::~ListControl() {
}

int ListControl::SelectedIndex() const {
  return model_ == NULL ? -1 : selected_index_;
}

void ListControl::SetModel(const ListModel* model) {
  model_ = model;
  selected_index_ = 0;

  Repaint();
  Emit(kEventSelectedItemChanged);
}

bool ListControl::AdjustSelectedIndex(int adjustment) {
  if (!model_) {
    return false;
  }

  const unsigned int new_index = selected_index_ + adjustment;

  if (!(new_index >= 0 && new_index < model_->Size())) {
    return false;
  }

  selected_index_ = new_index;

  Repaint();
  Emit(kEventSelectedItemChanged);

  return true;
}

unsigned int ListControl::SelectedRowIndex(WINDOW* window) const {
  const unsigned int scroll_line_index = Rows() / 2;

  unsigned int selected_row_index = 0;

  if (selected_index_ < scroll_line_index ||
      model_->Size() <= (unsigned int)Rows()) {
    selected_row_index = selected_index_;
  } else if (selected_index_ >= scroll_line_index &&
      selected_index_ <=
      (model_->Size() - (Rows() - scroll_line_index))) {
    selected_row_index = scroll_line_index;
  } else {
    int last_index_at_scroll =
      model_->Size() - (Rows() - scroll_line_index);

    selected_row_index =
      scroll_line_index + selected_index_ - last_index_at_scroll;
  }

  return selected_row_index;
}

bool ListControl::SelectPrevious() {
  return AdjustSelectedIndex(-1);
}

bool ListControl::SelectNext() {
  return AdjustSelectedIndex(1);
}

// virtual
int ListControl::PreferredHeight() const {
  return 5;
}

// virtual
void ListControl::PaintEvent() {
  WINDOW* window = Window();

  wclear(window);
  wnoutrefresh(window);

  if (!model_ || model_->Size() == 0) {
    return;
  }

  const int selected_row_index = SelectedRowIndex(window);

  const int first_index = selected_index_ - selected_row_index;

  const int last_index = min(static_cast<int>(model_->Size() - 1),
                                  first_index + Rows());

  int row = 0;
  for (int i = first_index; i <= last_index; ++i, ++row) {
    PaintLine(i, row, row == selected_row_index);
  }

  wnoutrefresh(window);
}

// virtual
void ListControl::PaintLine(unsigned int index, unsigned int row,
    bool selected) {
  WINDOW* window = Window();

  if (selected) {
    wattron(window, Colours::Get(Colours::kColourHighlighted) | A_BOLD);
  }

  wmove(window, row, 0);
  wprintw(window, "%3d  %-*s", index + 1, Cols() - 5,
      model_->Name(index).c_str());

  if (selected) {
    wattroff(window, Colours::Get(Colours::kColourHighlighted) | A_BOLD);
  }
}
}  // namespace x509ls

