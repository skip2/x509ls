// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_BASE_BASE_OBJECT_H_
#define X509LS_BASE_BASE_OBJECT_H_

#include <set>

#include "x509ls/base/types.h"

using std::set;

namespace x509ls {
class CliApplication;

class BaseObject {
 public:
  // Construct a BaseObject, with |parent|. Becomes owned by |parent|, must be
  // allocated on the stack.
  explicit BaseObject(BaseObject* parent);

  // Construct a BaseObject, with parent |application|. Becomes owned by
  // |application|, must be allocated on the stack.
  explicit BaseObject(CliApplication* application);

  virtual ~BaseObject();

  // Handle simple events posted from |source| with event code |event_code|.
  //
  // |event_code|s are arbitrary and specific to the |source|.
  virtual void OnEvent(const BaseObject* source, int event_code);

  // Handle the file descriptor event on |fd|. One or more of |read_event|,
  // |write_event|, |error_event| are set to true, depending on the events
  // occuring and event subscriptions.
  virtual void OnFDEvent(int fd, bool read_event,
      bool write_event, bool error_event);

  // Called regularly when the object has requested polling.
  virtual void OnPoll();

  // Returns the parent BaseObject or NULL.
  BaseObject* GetParent() const;

  // Returns the application object.
  CliApplication* GetApplication() const;

 protected:
  // Event handling.

  // Subscribe to events from |source| with event code |event_code|. Event codes
  // should not be negative.
  //
  // Events are delivered via OnEvent() until Unsubscribe() is called, or until
  // |source| is deleted.
  void Subscribe(BaseObject* source, int event_code);

  // Unsubscribe to events from |source| with event code |event_code|. If
  // |event_code| is -1, unsubscribes from all events.
  void Unsubscribe(BaseObject* source, int event_code = -1);

  // Emit an event with event code |event_code|.
  //
  // Event delivery is delayed until control flow returns to the top level event
  // loop.
  void Emit(int event_code);

  void WatchFD(int fd, int fd_events);
  void UnwatchFD(int fd);

  void EnablePoll();
  void DisablePoll();

  void DeleteChild(BaseObject* child);

 private:
  NO_COPY_AND_ASSIGN(BaseObject)

  void AddChild(BaseObject* child);

  BaseObject* parent_;
  CliApplication* application_;

  set<BaseObject*> children_;
};
}  // namespace x509ls

#endif  // X509LS_BASE_BASE_OBJECT_H_

