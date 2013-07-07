// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_BASE_COMMAND_LINE_H_
#define X509LS_CLI_BASE_COMMAND_LINE_H_

#include <string>

#include "x509ls/base/types.h"
#include "x509ls/cli/base/cli_control.h"

using std::string;

namespace x509ls {
// Text entry (single line & ASCII only) CLI control.
//
// Allows single line text entry and simple editing. Can alternatively display a
// read only message instead. Has a fixed height of one row.
//
// There are two modes of operation:
// - DisplayMessage: display a read-only message.
// - DisplayPrompt: display a read-only prompt, accept user-typed text.
//
// In the latter mode, the keyboard strokes recognised are:
// - ASCII keys: add text
// - Backspace
// - Delete
// - Enter: submit text
// - Left arrow key
// - Right arrow key
// - Home
// - End
//
// Two events may be emitted:
// - kEventInputAccepted: user pressed enter
// - kEventInputCancelled: user held the backspace key, "deleting" the prompt.
class CommandLine : public CliControl {
 public:
  explicit CommandLine(CliControl* parent);
  virtual ~CommandLine();

  // Returns 1.
  virtual int PreferredHeight() const;

  // Display |message_text| in the command line line only.
  //
  // Text entry is disabled. Any existing input text is cleared.
  void DisplayMessage(const string& message_text);

  // Enable text entry, displaying |prompt_question| as the question.
  //
  // For the |prompt_question| "Filename:", the screen will look like:
  // Filename: <user typed text appears here>
  //
  // Two events, kEventInputAccepted and kEventInputCancelled are emitted when
  // the user presses enter or cancels text entry respectively. After the
  // kEventInputAccepted event, the prompt and input text remains on the screen
  // with text entry disabled. Call Clear() or DisplayMessage() to remove it.
  void DisplayPrompt(const string& prompt_question,
      const string& input_text = "");

  // Clear the display, disable text entry and empty the user-input text buffer.
  void Clear();

  // Return the contents of the user-input text buffer.
  string InputText() const;

  // Events emitted during user text entry.
  enum Events {
    kEventInputAccepted,
    kEventInputCancelled
  };

 protected:
  virtual void PaintEvent();
  virtual bool KeyPressEvent(int keypress);

 private:
  NO_COPY_AND_ASSIGN(CommandLine)

  string prompt_text_;
  string input_text_;
  unsigned int cursor_offset_;
  bool is_accepting_input_text_;

  void PositionCursor();
};
}  // namespace x509ls

#endif  // X509LS_CLI_BASE_COMMAND_LINE_H_

