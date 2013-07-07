// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_BASE_CLI_APPLICATION_H_
#define X509LS_CLI_BASE_CLI_APPLICATION_H_

#include <ncurses.h>
#include <panel.h>

#include <list>
#include <string>

#include "x509ls/base/event_manager.h"
#include "x509ls/base/types.h"
#include "x509ls/cli/base/cli_control.h"

using std::list;
using std::string;

namespace x509ls {
// Abstract base class for an ncurses application.
//
// CliApplication provides the base for an ncurses application. It consists of:
// - A run loop oriented around keyboard input.
// - Ability to display multiple screens (arranged as a stack, like modal
//   dialogs).
// - Access to a publish/subscribe method for emitting and receiving simple
//   events.
//
// Classes implementing CliApplication should implement at least one method:
// RunEvent(). This is called after the run loop starts, and is intended to be
// used to setup the first screen.
//
// Use of a basic CliApplication is simply:
//   MyApplication my_app;
//   bool success = my_app.Run();
class CliApplication {
 public:
  CliApplication();
  virtual ~CliApplication();

  // Main run loop.
  //
  // The main program control flow. It has several steps:
  // - Setup and start displaying an ncurses interface.
  // - Call RunEvent() - abstract, must be implemented.
  // - Execute the run loop until Exit() is called, or the last screen layout is
  // closed. Delivers keyboard, CLI events, FD events, terminal resize events as
  // necessary.
  // - Call ExitEvent().
  // - Tear down and stop displaying the ncurses interface.
  //
  // Returns true iif the application run was deemed to be successful. Set by
  // the |success| flag to Exit().
  bool Run();

  // Request exit of the CLI during the run loop.
  //
  // The run loop will exit as soon as possible. |success| indicates whether the
  // application run was successful.
  void Exit(bool success);

  // Methods for showing and closing screen layouts. A screen layout is a
  // CliControl with the width and height of the terminal. Generally a screen
  // layout CliControl will contain child controls, and thus the screen will be
  // split into distinct controls, such as a menu bar and text input area.

  // Display |control| as a screen layout.
  //
  // |control| is added to the stack of screen layouts and displayed. |control|
  // must be allocated on the heap (new CliControl(...)): Ownership is
  // transfered here. |control| will be deleted when it is Close()'d, or
  // otherwise when the run loop exits.
  void Show(CliControl* control);

  // Close the screen layout |control|.
  //
  // The screen layout |control| is removed from the stack of screen layouts and
  // deleted. The next screen layout in the stack is then displayed.
  //
  // Close() requests are not processed immediately, rather they are delayed
  // until control returns to the top level event loop. This enables a |control|
  // to request closing (and thus deleting) of itself.
  //
  // If no more screen layouts exist then the run loop exits, returning false.
  void Close(CliControl* control);

  // Return the application's EventManager.
  EventManager* GetEventManager();

  // Returns the currently focused CliControl, or NULL if no control currently
  // has focus.
  CliControl* FocusedControl() const;

 protected:
  // Called during Run() to enable initial screen layout setup.
  virtual void RunEvent() = 0;

  // Called during Run() to enable final teardown.
  virtual void ExitEvent();

 private:
  NO_COPY_AND_ASSIGN(CliApplication)

  // Flag to indicate if the run loop should exit.
  bool exit_requested_;

  // Flag to indicate if the application run has been successful or not.
  bool exit_success_;

  // Startup the ncurses environment.
  void StartNCurses();

  // Stop and tear down the ncurses environment.
  void StopNCurses();

  // ncurses terminal data structure.
  SCREEN* screen_;

  // Each screen layout is implemented using an ncurses panel and a top level
  // window within that panel. struct Layout associates the |panel| with the top
  // level |window| and top level |control|.
  struct Layout {
    PANEL* panel;
    WINDOW* window;
    CliControl* control;
    Layout(PANEL* panel_, WINDOW* window_, CliControl* control_)
      :
        panel(panel_),
        window(window_),
        control(control_) {
    }
  };
  // List of screen layouts.
  list<struct Layout> layouts_;

  // List of CliControls requested to be closed.
  list<CliControl*> deferred_close_requests_;

  // Closes any requested CliControls.
  void ProcessDeferredCloseRequests();

  // Resizes all existing screen layouts in response to the terminal being
  // resized.
  void ResizeAll();

  // Access via GetEventManager().
  EventManager event_manager_;
};
}  // namespace x509ls

#endif  // X509LS_CLI_BASE_CLI_APPLICATION_H_

