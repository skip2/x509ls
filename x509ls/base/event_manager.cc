// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/base/event_manager.h"

#include <assert.h>
#include <sys/select.h>

#include <algorithm>
#include <utility>

#include "x509ls/base/base_object.h"

using std::max;
using std::pair;

namespace x509ls {
// static
const int EventManager::kFDReadable = 0x1;

// static
const int EventManager::kFDWritable = 0x2;

// static
const int EventManager::kFDException = 0x4;

// static
const int EventManager::kFDAllEvents =
  EventManager::kFDReadable |
  EventManager::kFDWritable |
  EventManager::kFDException;

EventManager::EventManager() {
}

EventManager::~EventManager() {
}

void EventManager::Register(BaseObject* control) {
  // For consistency.
}

void EventManager::Unregister(const BaseObject* control) {
  // Erase subscriptions.
  multimap<struct Event, BaseObject*>::iterator it;
  for (it = subscriptions_.begin(); it != subscriptions_.end();) {
    if (it->first.source == control || it->second == control) {
      multimap<struct Event, BaseObject*>::iterator it2 = it;
      it++;
      subscriptions_.erase(it2);
    } else {
      it++;
    }
  }

  // Erase any outstanding queued events.
  list<struct Event>::iterator qit;
  for (qit = queued_events_.begin(); qit != queued_events_.end();) {
    if (qit->source == control) {
      qit = queued_events_.erase(qit);
    } else {
      ++qit;
    }
  }

  // Erase any watched FDs.
  multimap<int, FDWatcher>::iterator wit;
  for (wit = watched_fds_.begin(); wit != watched_fds_.end();) {
    if (wit->second.receiver == control) {
      multimap<int, FDWatcher>::iterator wit2 = wit;
      wit++;
      watched_fds_.erase(wit2);
    } else {
      wit++;
    }
  }

  // Disable receiving poll events.
  poll_receivers_.erase(const_cast<BaseObject*>(control));
}

void EventManager::Subscribe(const BaseObject* source,
    BaseObject* destination,
    int event_code) {
  const struct Event event(source, event_code);

  pair<multimap<struct Event, BaseObject*>::iterator,
    multimap<struct Event, BaseObject*>::iterator> range;
  range = subscriptions_.equal_range(event);

  multimap<struct Event, BaseObject*>::iterator it;
  for (it = range.first; it != range.second; it++) {
    if (it->second == destination) {
      return;
    }
  }

  subscriptions_.insert(
      pair<struct Event, BaseObject*>(event, destination));
}

void EventManager::Unsubscribe(const BaseObject* source,
    const BaseObject* destination,
    int event_code) {
  multimap<struct Event, BaseObject*>::iterator it;
  for (it = subscriptions_.begin(); it != subscriptions_.end();) {
    if (it->first.source == source && it->second == destination &&
       (event_code == -1 || it->first.event_code == event_code)) {
      subscriptions_.erase(it++);
    } else {
      it++;
    }
  }
}

void EventManager::DeliverEvents() {
  while (queued_events_.size() > 0) {
    const struct Event event = queued_events_.front();
    queued_events_.pop_front();

    // Take a copy of subscriptions_ here: Event handler code may
    // cause it to be modified.
    multimap<struct Event, BaseObject*> subs_to_deliver = subscriptions_;

    pair<multimap<struct Event, BaseObject*>::const_iterator,
      multimap<struct Event, BaseObject*>::const_iterator> range;
    range = subs_to_deliver.equal_range(event);

    multimap<struct Event, BaseObject*>::const_iterator it;
    for (it = range.first; it != range.second; ++it) {
      // TODO(tfh): check destination & source still in set of valid objects
      // before dispatching here.
      it->second->OnEvent(event.source, event.event_code);
    }
  }
}

void EventManager::EnqueueEvent(const BaseObject* source,
    const int event_code) {
  const struct Event event(source, event_code);
  queued_events_.push_back(event);
}

void EventManager::WatchFD(BaseObject* destination, int fd, int fd_events) {
  if (!fd_events) {
    UnwatchFD(destination, fd);
  }

  pair<multimap<int, FDWatcher>::iterator,
    multimap<int, FDWatcher>::iterator> range;
  range = watched_fds_.equal_range(fd);

  multimap<int, FDWatcher>::iterator it;
  for (it = range.first; it != range.second; it++) {
    if (it->second.receiver == destination) {
      it->second.fd_events = fd_events;
      return;
    }
  }

  const FDWatcher new_fd_watch(destination, fd_events);

  watched_fds_.insert(pair<int, FDWatcher>(fd, new_fd_watch));
}

void EventManager::UnwatchFD(BaseObject* destination, int fd) {
  multimap<int, FDWatcher>::iterator wit;
  for (wit = watched_fds_.begin(); wit != watched_fds_.end(); ++wit) {
    if (wit->first == fd && wit->second.receiver == destination) {
      watched_fds_.erase(wit);
      break;
    }
  }
}

bool EventManager::HasNetworkEvents() const {
  return watched_fds_.size() > 0 || poll_receivers_.size() > 0;
}

void EventManager::DeliverNetworkEvents(int timeout_ms) {
  DeliverFDEvents(timeout_ms);
  DeliverPoll();
}

void EventManager::DeliverFDEvents(int timeout_ms) {
  // Timeout setup.
  const int kMillisecondsInAMicrosecond = 1000;
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = timeout_ms * kMillisecondsInAMicrosecond;

  const int kFDSetCount = 3;
  fd_set fds[kFDSetCount];

  for (int i = 0; i < kFDSetCount; ++i) {
    FD_ZERO(&fds[i]);
  }

  // Setup FDs to be watched.
  multimap<int, FDWatcher>::const_iterator wit;
  int highest_fd = -1;
  for (wit = watched_fds_.begin(); wit != watched_fds_.end(); ++wit) {
    assert(wit->second.fd_events & kFDAllEvents);

    highest_fd = max(highest_fd, wit->first);

    if (wit->second.fd_events & kFDReadable) {
      FD_SET(wit->first, &fds[0]);
    }

    if (wit->second.fd_events & kFDWritable) {
      FD_SET(wit->first, &fds[1]);
    }

    if (wit->second.fd_events & kFDException) {
      FD_SET(wit->first, &fds[2]);
    }
  }

  int ready_descriptor_count = select(highest_fd + 1,
     &fds[0], &fds[1], &fds[2], &timeout);

  if (ready_descriptor_count <= 0) {
    return;
  }

  // Emit events.
  // Uses a copy of watched_fds_ since it may be changed by event receivers.
  multimap<int, FDWatcher> watched_fds = watched_fds_;
  for (wit = watched_fds.begin(); wit != watched_fds.end(); ++wit) {
    bool read_event = FD_ISSET(wit->first, &fds[0]) != 0 &&
      wit->second.fd_events & kFDReadable;

    bool write_event = FD_ISSET(wit->first, &fds[1]) != 0 &&
      wit->second.fd_events & kFDWritable;

    bool exception_event = FD_ISSET(wit->first, &fds[2]) != 0 &&
      wit->second.fd_events & kFDException;

    if (read_event || write_event || exception_event) {
      wit->second.receiver->OnFDEvent(wit->first,
          read_event, write_event, exception_event);
    }
  }
}

void EventManager::EnablePoll(BaseObject* destination) {
  poll_receivers_.insert(destination);
}

void EventManager::DisablePoll(BaseObject* destination) {
  poll_receivers_.erase(destination);
}

void EventManager::DeliverPoll() {
  set<BaseObject*> poll_receivers = poll_receivers_;

  for (set<BaseObject*>::iterator it = poll_receivers.begin();
      it != poll_receivers.end();
      ++it) {
    (*it)->OnPoll();
  }
}
}  // namespace x509ls

