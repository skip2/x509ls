// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/cli/base/cli_application.h"

#include <locale.h>
#include <stdio.h>

namespace x509ls {
CliApplication::CliApplication()
  :
    exit_requested_(false),
    exit_success_(false) {
}

CliApplication::~CliApplication() {
}

bool CliApplication::Run() {
  exit_requested_ = false;
  exit_success_ = false;

  StartNCurses();

  RunEvent();

  ProcessDeferredCloseRequests();
  event_manager_.DeliverEvents();

  doupdate();

  while (!exit_requested_ && layouts_.size() > 0) {
    // The event loop watches two types of events:
    // - file descriptors (e.g. network socket)
    // - ncurses's getch() events (can be set blocking/non-blocking).
    //
    // There are two modes of operation, depending on whether network activity
    // is in progress:
    //
    // - keyboard only mode (no FDs or polls) - Blocks on ncurses getch().
    // - network mode (watching FDs or has polls) - Blocks watching FDs, with a
    // timeout every 50ms for a non-blocking getch(). Calls an OnPoll() event
    // on objects requesting it. This allows network traffic to run without
    // delay, but still allows for keyboard intervention (e.g. supports a
    // keypress to cancel the network activity).

    bool using_network_events = event_manager_.HasNetworkEvents();

    if (using_network_events) {
      event_manager_.DeliverNetworkEvents(250);
    }

    // getch() setup: is_watching_fds ? 1ms timeout : blocking
    timeout(using_network_events ? 1 : -1);

    int ch = getch();
    if (ch == ERR) {
      // Ignore non-blocking no-character-available condition.
    } else if (ch == KEY_RESIZE) {
      ResizeAll();
    } else if (layouts_.size() > 0) {
      layouts_.front().control->OnKeyPress(ch);
    } else {
      Exit(false);
    }

    ProcessDeferredCloseRequests();
    event_manager_.DeliverEvents();

    doupdate();
  }

  while (layouts_.size() > 0) {
    Close(layouts_.front().control);
  }

  ExitEvent();

  StopNCurses();

  return exit_success_;
}

void CliApplication::Exit(bool success) {
  exit_requested_ = true;
  exit_success_ = success;
}

void CliApplication::StartNCurses() {
  setlocale(LC_ALL, "en_GB.UTF-8");

  screen_ = newterm(NULL, stdout, stdin);

  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  start_color();
  curs_set(0);

  refresh();
}

void CliApplication::StopNCurses() {
  echo();

  endwin();
  delscreen(screen_);
}

// virtual
void CliApplication::ExitEvent() {
}

void CliApplication::ResizeAll() {
  int rows, cols;
  getmaxyx(stdscr, rows, cols);

  for (list<Layout>::iterator it = layouts_.begin();
      it != layouts_.end();
      ++it) {
    WINDOW* new_window = newwin(rows, cols, 0, 0);

    replace_panel(it->panel, new_window);
    it->control->SetWindow(new_window);

    delwin(it->window);
    it->window = new_window;

    it->control->Repaint();
  }

  update_panels();
  doupdate();

  FocusedControl()->OnFocus();
}

void CliApplication::Show(CliControl* main_control) {
  int rows, cols;
  getmaxyx(stdscr, rows, cols);

  WINDOW* window = newwin(rows, cols, 0, 0);
  PANEL* panel = new_panel(window);

  const struct Layout new_layout(panel, window, main_control);
  layouts_.push_front(new_layout);
  main_control->SetWindow(window);

  update_panels();
  doupdate();
  refresh();

  main_control->Repaint();
}

void CliApplication::Close(CliControl* control) {
  deferred_close_requests_.push_back(control);
}

void CliApplication::ProcessDeferredCloseRequests() {
  bool processed_any_close_requests = false;

  for (list<CliControl*>::const_iterator cit = deferred_close_requests_.begin();
      cit != deferred_close_requests_.end();
      ++cit) {
    for (list<struct Layout>::iterator it = layouts_.begin();
        it != layouts_.end(); ++it) {
      if (it->control == *cit) {
        processed_any_close_requests = true;

        del_panel(it->panel);

        if (it->window) {
          it->control->SetWindow(NULL);
          delwin(it->window);
        }

        delete it->control;

        layouts_.erase(it);

        break;
      }
    }
  }

  if (processed_any_close_requests) {
    deferred_close_requests_.clear();

    if (layouts_.size() > 0) {
      layouts_.front().control->Repaint();
    }

    update_panels();
    doupdate();
  }
}

EventManager* CliApplication::GetEventManager() {
  return &event_manager_;
}

CliControl* CliApplication::FocusedControl() const {
  if (layouts_.size() == 0) {
    return NULL;
  }

  CliControl* current = layouts_.front().control;
  while (current && current->FocusedChild() != NULL) {
    current = current->FocusedChild();
  }

  return current;
}

}  // namespace x509ls

