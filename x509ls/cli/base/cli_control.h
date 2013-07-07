// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_BASE_CLI_CONTROL_H_
#define X509LS_CLI_BASE_CLI_CONTROL_H_

#include <ncurses.h>
#include <stddef.h>

#include <list>
#include <string>

#include "x509ls/base/base_object.h"
#include "x509ls/base/types.h"

using std::list;
using std::string;

namespace x509ls {
class CliApplication;
// Base class for CliControls.
//
// A CliControl occupies a region of screen space, typically one or more rows.
// Like GUI controls it can receive keyboard events, gain and lose focus, and
// emit/receive simple events. The PaintEvent() method is overridden to "paint"
// characters onto the screen.
//
// A typical application is structured as follows:
//
//      *        CliApplication.
//     / \       .
//    *   *      Top level CliControls (screen layouts).
//   /   / \     .
//  *   *   *    Child CliControls.
//         / \   .
//        *   *  More Child CliControls.
//
// There are a couple of ways to use CliControl:
//
// - A simple CliControl (no child controls). Used to display text on the
//   screen, possibly respond to keyboard events. e.g. a text editing control.
// - A composite CliControl (has child controls). Used to help display multiple
//   child CliControls in a layout. e.g. display a menu bar, text viewer and
//   commandbar. The simple built-in layout manager assigns a number of rows to
//   each child control as determined by their PreferredHeight()s.
//
// A top level screen layout CliControl likely has child controls, otherwise
// only one control is displayed.
//
// CliControls must be allocated on the heap (new CliControl(...), and become
// owned by their parent CliControl (or CliApplication for top level controls).
// When a CliControl is deleted, its child controls are deleted at the same
// time, creating hierarchical ownership of controls. Deleting a top level
// application thus results in a tidy recursive deletion of all child objects.
class CliControl : public BaseObject {
 public:
  // Construct a CliControl with parent |parent|. Must be allocated on the heap
  // and becomes owned by |parent|.
  explicit CliControl(CliControl* parent);

  // Construct a CliControl with parent application |application|. Use for top
  // level CliControls only. Becomes owned by |application|.
  explicit CliControl(CliApplication* parent);

  virtual ~CliControl();

  // Return the preferred height in rows.
  //
  // Return -1 to request using the maximum space available. By default returns
  // -1.
  virtual int PreferredHeight() const;

  // Repaint the control and any child controls.
  //
  // |recursive_repaint| should always be false. It is used internally to limit
  // restoration of focus to the top level call.
  void Repaint(bool recursive_repaint = false);

  // Handle the ncurses keypress |keypress|.
  //
  // Returns true iif the keypress was handled by the CliControl or a child
  // control.
  //
  // Keypress events follow an "event bubbling" model. They are first offered to
  // the most inner focused child control, which can indicate (by returning
  // true/false) if the keypress was handled.
  //
  // Unhandled events are passed up to parent CliControls until handled.
  // Remaining keypress events are discarded.
  bool OnKeyPress(int keypress);

  // Handle terminal resize events.
  //
  void OnResize(WINDOW* new_window);

  // Set the window to occupy to |window|.
  //
  // |window| may be NULL if insufficient terminal space exists, or during
  // object deletion. Called when the terminal is resized.
  //
  // |window| is the new window space to accept. Recursively creates new windows
  // for any child controls and calls SetWindow() on them.
  void SetWindow(WINDOW* window);

  // Called when focus is gained. Also called on a focused control after another
  // control completes painting. Sets terminal cursor location & visibility.
  void OnFocus();

  // Called when focus is lost. Hides the terminal cursor.
  void OnBlur();

  // Return the current child CliControl selected for focus, or otherwise NULL
  // to retain focus.
  //
  // This does not necessarily indicate the CliControl with actual screen focus:
  // There must be a chain of focus from the CliApplication (displaying the
  // appropriate screen layout) down to the required object.
  CliControl* FocusedChild();

  // The following methods return the size of the assigned Window(), into which
  // all painting should be performed. They return 0 if no window is currently
  // assigned.
  int Rows() const;
  int Cols() const;

 protected:
  // Handle keypress event |keypress|. Returns true iif the keypress event was
  // handled.
  virtual bool KeyPressEvent(int keypress);

  // Handle a paint event. The control must paint its entire window.
  virtual void PaintEvent();

  // Handle a resize event.
  virtual void ResizeEvent();

  // Return the current ncurses window. Remains owned by the parent CliControl
  // or CliApplication. May be NULL.
  WINDOW* Window() const;

  // Add CliControl |control| to be displayed as a sub-control.
  // Takes ownership of |control| and responsible for its destruction.
  void AddChild(CliControl* control);

  // Replace the sub-control at |index| (previously added with AddChild) with
  // |control|.
  //
  // Used for swapping one control for another. There are no changes in
  // ownership to either control.
  void ReplaceChild(int index, CliControl* control);

  // Set the currently focused child control to |control|. Set to NULL to take
  // focus.
  void SetFocusedChild(CliControl* control);

  // Cursor handling.
  // The cursor state is set here. The cursor will only be displayed when the
  // control has focus. State is retained, so the cursor is shown/hidden as the
  // control gains/loses focus as necessary.
  //
  // Move the cursor to the position (|y|, |x|) relative to the control's
  // window.
  void SetCursor(unsigned int y, unsigned int x);

  // Hide the cursor.
  void HideCursor();

 private:
  NO_COPY_AND_ASSIGN(CliControl)

  CliControl* parent_;

  // Access with FocusedChild().
  CliControl* focused_child_;

  // Indicates whether the control has
  bool has_focus_;

  // Access with Window().
  WINDOW* my_window_;

  // Associates a child CliControl with the window provided to it (may be NULL),
  // and the number of terminal rows allocated to it (may be -1 to indicate
  // using to use the maximum space available).
  struct ChildControl {
    CliControl* control;
    WINDOW* window;
    int required_rows;
    ChildControl(CliControl* control_, WINDOW* window_, int required_rows_)
      :
        control(control_),
        window(window_),
        required_rows(required_rows_) {
    }
  };
  list<ChildControl> children_;

  // Cursor handling.
  bool show_cursor_;
  unsigned int cursor_y;
  unsigned int cursor_x;
  void UpdateCursor();

  // Cached row/column size of Window(), set when a window is assigned/removed
  // using SetWindow().
  int rows_;
  int cols_;
};
}  // namespace x509ls

#endif  // X509LS_CLI_BASE_CLI_CONTROL_H_

