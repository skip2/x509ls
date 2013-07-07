// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_BASE_TEXT_CONTROL_H_
#define X509LS_CLI_BASE_TEXT_CONTROL_H_

#include <ncurses.h>

#include <map>
#include <string>

#include "x509ls/base/types.h"
#include "x509ls/cli/base/cli_control.h"

using std::map;
using std::string;

namespace x509ls {
// Scrollable text CLI control.
//
// A CLI control for displaying multiple lines of text, including text larger
// than the available display area. Lines of text wider than the display area
// are wrapped onto new lines.
//
// The text may be scrolled by both method call (Scroll()), and by keyboard
// events. The recognised keys are:
//
// - Down arrow: scroll one line down.
// - Up arrow: scroll one line up.
// - Page Down:  Scroll one page down.
// - Page Up:    Scroll one page up.
class TextControl : public CliControl {
 public:
  // Construct a TextControl to display |text|.
  explicit TextControl(CliControl* parent, const string& text);
  ~TextControl();

  // Directions text may be scrolled.
  enum ScrollDirection {
    kDirectionUp,
    kDirectionDown
  };

  // Scroll the text in |direction| by |lines|.
  //
  // Scrolls by one page if |lines| is 0.
  //
  // Returns true iif the the text could be scrolled in |direction|. Call
  // Render() after scrolling to update the display.
  bool Scroll(const enum ScrollDirection direction, int lines = 0);

  // Set the displayed text to |text|.
  //
  // The scroll offset is reset to zero and the new text is painted.
  void SetText(const string& text);

 protected:
  virtual void PaintEvent();
  virtual bool KeyPressEvent(int keypress);

 private:
  NO_COPY_AND_ASSIGN(TextControl)

  WINDOW* pad_;

  string text_;

  unsigned int first_visible_line_index_;
  map<unsigned int, unsigned int> line_to_byte_offset_;
  unsigned int line_count_;
};
}  // namespace x509ls

#endif  // X509LS_CLI_BASE_TEXT_CONTROL_H_

