// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_BASE_EVENT_MANAGER_H_
#define X509LS_BASE_EVENT_MANAGER_H_

#include <list>
#include <map>
#include <set>

#include "x509ls/base/types.h"

using std::list;
using std::multimap;
using std::set;

namespace x509ls {
class BaseObject;
// Provide three event mechanisms:
// - A simple publish-subscribe event manager, to enable decoupled objects (such
//   as reusable CLI controls, think GUI controls) to signal simple events such
//   as "text entry complete".
// - A mechanism for watching file descriptor events.
// - A mechanism for requesting an object method be repeatedly polled.
//
// Used as part of an external event loop.
//
// The pub-sub event manager works as follows: Objects deriving from BaseObject
// may emit simple events (arbitrary event codes, and subscribe to selected
// events. When an event occurs, a method is called on each of the subscribing
// BaseObjects. A typical example is:
//
// - A CliControl (a BaseObject child class) called MyLayout creates a child
//   TextView.
// - MyLayout subscribes to kTextEntryFinished and kTextEntryCancelled events on
//   the TextView.
// - User types some text and hits enter. The TextView emits a
//   kTextEntryFinished event.
// - An event callback on MyLayout occurs with the kTextEntryFinishedEvent.
//
// This could be achieved directly by method calling, however EventManager has
// the advantages:
// - Event delivery is deferred by default. This ensures events are only
//   delivered when the source BaseObjects is in a consistent state.
// - Multiple objects can subscribe to the same event.
// - Subscriptions are automatically finished when objects are destroyed.
//
// An application (only example so far is a CliApplication) should have
// precisely one EventManager and call DeliverEvents() as part of its run loop.
class EventManager {
 public:
  EventManager();
  ~EventManager();

  // The following two methods provide unobtrusive reference counting for
  // objects publishing/subscribing via EventManager. They are called by the
  // BaseObject base class.

  // Register |object| to send and receive events.
  //
  // Call when |object| is constructed.
  void Register(BaseObject* object);

  // Unregister |object| from sending and receiving events.
  //
  // Call just before |object| is destroyed.
  //
  // Any event subscriptions for |object| are removed. Any outstanding events
  // to be delivered from |object| are removed and not delivered.
  void Unregister(const BaseObject* object);

  // Schedule an event from |source| with |event_code| to be delivered to any
  // subscribers. |event_code| is arbitrary and specific to |source|.
  void EnqueueEvent(const BaseObject* source, const int event_code);

  // Deliver pub-sub events until no events are outstanding.
  void DeliverEvents();

  // Subscribe to events from |source| with |event_code|, to be delivered to
  // |destination|.
  void Subscribe(const BaseObject* source,
      BaseObject* destination,
      int event_code);

  // Unsubscribe to events from |source| with |event_code| previously delivered
  // to |destination|. Set |event_code| to -1 to unsubscribe from all events.
  void Unsubscribe(const BaseObject* source,
      const BaseObject* destination,
      int event_code = -1);

  // ---------------------------------------------------------------------------
  // Methods for watching file descriptors.

  // Flags to indicate which FD events should be signaled, may be ORed together.
  static const int kFDReadable;
  static const int kFDWritable;
  static const int kFDException;
  static const int kFDAllEvents;

  // Watch the file descriptor |fd| for the events specified by |fd_events|
  // (e.g. kFDReadable | kFDWritable), and deliver events to |destination|.
  //
  // During event delivery (DeliverNetworkEvents(), OnFDEvent is called on
  // on |destination| when the specified event(s) occur.
  //
  // Each |destination| has one watch of |fd|. Multiple calls to WatchFD() with
  // the same |destination| and |fd| updates the chosen |fd_events|, rather than
  // adding a new watch. Setting |fd_events| to 0 removes the watch.
  void WatchFD(BaseObject* destination, int fd, int fd_events);

  // Remove watch of |fd| by |destination|.
  void UnwatchFD(BaseObject* destination, int fd);

  // Return true iif any FDs are being watched, or any objects are receiving
  // regular polling events.
  bool HasNetworkEvents() const;

  // Deliver watched FD & poll events.
  //
  // Wait upto |timeout_ms| milliseconds for network activity before timing out.
  void DeliverNetworkEvents(int timeout_ms = 100);

  // Enable and disable polling on |destination|.
  void EnablePoll(BaseObject* destination);
  void DisablePoll(BaseObject* destination);

 private:
  NO_COPY_AND_ASSIGN(EventManager)

  struct Event {
    const BaseObject* source;
    int event_code;
    Event(const BaseObject* source_, int event_code_)
      :
        source(source_), event_code(event_code_) {
    }

    bool operator<(const struct Event& other) const {
      return source < other.source ||
        (source == other.source && event_code < other.event_code);
    }

    bool operator==(const struct Event& other) const {
      return source == other.source && event_code == other.event_code;
    }

    bool operator!=(const struct Event& other) const {
      return !(*this == other);
    }
  };
  list<struct Event> queued_events_;

  multimap<struct Event, BaseObject*> subscriptions_;

  struct FDWatcher {
    BaseObject* receiver;
    int fd_events;

    FDWatcher(BaseObject* receiver_,
        int fd_events_)
      :
        receiver(receiver_),
        fd_events(fd_events_) {
    }
  };
  multimap<int, FDWatcher> watched_fds_;

  set<BaseObject*> poll_receivers_;

  void DeliverFDEvents(int timeout_ms);
  void DeliverPoll();
};
}  // namespace x509ls

#endif  // X509LS_BASE_EVENT_MANAGER_H_

