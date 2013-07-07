// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_NET_DNS_LOOKUP_H_
#define X509LS_NET_DNS_LOOKUP_H_

// For getaddrinfo_a(3).
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <netdb.h>

#include <string>

#include "x509ls/base/base_object.h"
#include "x509ls/base/types.h"

using std::string;

namespace x509ls {
// Perform an asynchronous DNS lookup.
//
// Resolves a single hostname using the system's default resolver. There are
// four types of DNS lookup:
//
// - kAddressFamilyIPv4: Lookup A record only
// - kAddressFamilyIPv6: Lookup AAAA record only
// - kAddressFamilyIPv4then6: Lookup A record first, then try AAAA second.
// - kAddressFamilyIPv6then4: Loookup AAAA record first, then try A second.
//
// Lookups are asynchronous, with events emitted when name resolution has
// succeeded or failed.
//
// For the kAddressFamilyIPvXthenX lookup types, success/failure refers to the
// final result, no events are emitted if the first lookup tried fails.
class DnsLookup : public BaseObject {
 public:
  enum AddressFamily {
    kAddressFamilyIPv4,
    kAddressFamilyIPv6,
    kAddressFamilyIPv4then6,
    kAddressFamilyIPv6then4
  };

  // Construct a DnsLookup with |parent|, |hostname| (which may be an IP address
  // string), |port|, and |address_family| lookup type.
  //
  // |port| is placed into the sockaddr struct of successful lookups.
  DnsLookup(BaseObject* parent, const string& hostname,
      uint16 port, AddressFamily address_family);

  // Leaks memory if HasOutstandingRequests() is true.
  virtual ~DnsLookup();

  // Start asynchronous DNS lookup.
  //
  // Call once only.
  //
  // Eventually one of two events will be Emit()ed:
  // - kStateSuccess: DNS lookups succeeded. The result is available from the
  // Sockaddr(), SockaddrLen() and IPAddress() methods.
  // - kStateFail: DNS lookups failed.
  void Start();

  // Cancel asynchronous DNS lookup.
  //
  // Attempts to cancel any outstanding DNS lookups. Only queued lookups (those
  // not yet queued) can be cancelled.
  void Cancel();

  // Returns true iif there are outstanding lookups. DnsLookup will leak a
  // small amount of memory if deleted while there are outstanding requests.
  bool HasOutstandingRequests() const;

  enum State {
    kStateStart,
    kStateInProgress,
    kStateSuccess,    // Emitted as an event.
    kStateFail        // Emitted as an event.
  };
  // Returns the current state.
  State GetState() const;

  // Called frequently by the application event loop while a DNS lookup is
  // running.
  //
  // The underlying asynchronous name resolution method (getaddrinfo_a(3))
  // provides a couple of different methods for signalling success/failure:
  // - a signal(7)
  // - can start a new thread
  // - can be polled to check for completion.
  //
  // In this case I've chosen the polling approach. In this small CLI
  // application, I chose polling because:
  // - simpler to implement
  // - performance isn't critical in this small CLI program
  // - use of signals would encourage the use of singletons
  // - easier not to interact with ncurses' use of signals.
  virtual void OnPoll();

  // The following methods are only valid when kStateSuccess is Emit()ed.

  // Returns the sockaddr struct of the successful lookup. The port is as
  // specified in the constructor.
  const sockaddr* Sockaddr() const;

  // Returns the length of the sockaddr struct.
  socklen_t SockaddrLen() const;

  // Returns a text representation of the IP address.
  string IPAddress() const;

  // Methods for choosing the AddressFamily.

  // Returns a short string describing |address_family|, suitable for displaying
  // to the user.
  static string AddressFamilyName(const AddressFamily& address_family);

  // Returns the next AddressFamily in the list after |address_family|.
  //
  // For the last AddressFamily in the list, the first AddressFamily is
  // returned. This enables a CLI control to easily and obliviously cycle
  // through the possible options continuously.
  // next |address_family|
  static AddressFamily NextAddressFamily(
      const AddressFamily& address_family);

  string ErrorMessage() const;

 private:
  NO_COPY_AND_ASSIGN(DnsLookup)

  const string hostname_;
  const uint16 port_;
  const enum AddressFamily address_family_;

  struct gaicb* request_ptrs[2];
  struct gaicb requests[2];
  struct addrinfo config[2];
  int request_count_;
  int result_index_;

  char* port_str_;

  void SetState(State state, bool emit_event = false);
  State state_;

  bool HasBuggyGlibc() const;

  string error_message_;
};
}  // namespace x509ls

#endif  // X509LS_NET_DNS_LOOKUP_H_

