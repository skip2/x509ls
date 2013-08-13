// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_BASE_LIST_CONTROL_H_
#define X509LS_CLI_BASE_LIST_CONTROL_H_

#include <stddef.h>

#include "x509ls/cli/base/cli_control.h"
#include "x509ls/cli/base/list_model.h"
#include "x509ls/base/types.h"

namespace x509ls {
// Scrollable, selectable list CLI control.
//
// A CLI control for displaying a list of items. Allows scrolling through the
// list of items, with one item selected at all times.
//
// By default a numbered line of text is displayed for each list item:
//
//   1 Item A
//   2 Item B
//   3 Item C
//
// Not designed for long lists, three characters are dedicated for the
// numbering.
//
// To display more complicated information (such as columns of information),
// inherit from ListControl and reimplement the PaintLine method.
class ListControl : public CliControl {
 public:
  // Construct a ListControl to display items from |model|.
  //
  // |model|'s items must remain constant while used by the ListControl.
  ListControl(CliControl* parent, const ListModel* model = NULL);
  virtual ~ListControl();

  // Return the index of the currently selected item in |model|. Returns -1 if
  // no item is selected, which only occurs when |model| is NULL or empty.
  int SelectedIndex() const;

  // Set the data source to |model|.
  //
  // |model|'s items must remain constant while used with ListControl. The
  // selected index is reset back to zero. Repaints to display the new model.
  void SetModel(const ListModel* model);

  // The following methods return true iif the selected index changed. Reasons
  // for not changing include selecting a negative or identical index.
  //
  // The following methods Repaint() as necessary.

  // Select the previous item. The index is decremented by 1.
  bool SelectPrevious();

  // Select the next item. The index is incremented by 1.
  bool SelectNext();

  // Select the first item.
  bool SelectFirst();

  // Select the last item.
  bool SelectLast();

  // Returns the preferred height of the control, default 5 rows.
  virtual int PreferredHeight() const;

  // Event emitted when the selected item is changed.
  static const int kEventSelectedItemChanged;

 protected:
  virtual void PaintEvent();
  virtual void PaintLine(unsigned int index, unsigned int row, bool selected);

 private:
  NO_COPY_AND_ASSIGN(ListControl)

  const ListModel* model_;
  unsigned int selected_index_;

  // Change the selected index by |adjustment| (typically 1 for down, -1 for
  // up).
  //
  // Returns true if the index could be changed (e.g. returns false if requested
  // to move off the end of the list).
  //
  // Repaints and emits the kEventSelectedItemChanged event as necessary.
  bool AdjustSelectedIndex(int adjustment);

  // For long lists, the list can be scrolled up and down. The control favours
  // keeping the selected item in the middle of the control, where possible. For
  // example, scrolling through a 10 item list:
  //   1. aaa [selected item]
  //   2. bbb
  //   3. ccc
  //   4. ddd
  //   5. eee
  // ...
  //   4. ddd
  //   5. eee
  //   6. fff [selected item]
  //   7. ggg
  //   8. hhh
  // ...
  //   5. eee
  //   6. fff
  //   7. ggg [selected item]
  //   8. hhh
  //   9. iii
  // ...
  //   6. fff
  //   7. ggg
  //   8. hhh
  //   9. iii
  //  10. jjj [selected item]
  //
  //  Given the selected index in the model, return the index of the screen row
  //  it should be displayed at, as above.
  unsigned int SelectedRowIndex(WINDOW* window) const;

  // Set the selected index to |index|. If |index| matches the current index,
  // returns false. Otherwise, sets the index, repaints the control, emits
  // kEventSelectedItemChanged, then returns true.
  bool SetSelectedIndex(unsigned int index);
};
}  // namespace x509ls

#endif  // X509LS_CLI_BASE_LIST_CONTROL_H_

