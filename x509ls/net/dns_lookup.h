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
// Resolves a single hostname using the system's default resolver. Also
// "resolves" IPv4 and IPv6 address strings. There are four types of DNS lookup:
//
// - kLookupTypeIPv4: Lookup A record only
// - kLookupTypeIPv6: Lookup AAAA record only
// - kLookupTypeIPv4then6: Lookup A record first, then try AAAA second.
// - kLookupTypeIPv6then4: Lookup AAAA record first, then try A second.
//
// Lookups are asynchronous, with events emitted when name resolution has
// succeeded or failed. For the kLookupTypeIPvXthenX lookup types,
// success/failure refers to the final result, no events are emitted if the
// first lookup tried fails.
//
// Uses glibc's asynchronous DNS lookup function getaddrinfo_a(). This function
// was buggy between about glibc versions 2.5-8. The glibc version is examined
// upon calling Start() and an error occurs if any of these versions are being
// used.
class DnsLookup : public BaseObject {
 public:
  enum LookupType {
    kLookupTypeIPv4,
    kLookupTypeIPv6,
    kLookupTypeIPv4then6,
    kLookupTypeIPv6then4
  };

  // Construct a DnsLookup with |parent|, |hostname| (which may be an IP address
  // string), |port|, and |lookup_type| lookup type.
  //
  // |port| is placed into the sockaddr struct of successful lookups.
  DnsLookup(BaseObject* parent, const string& hostname,
      uint16 port, LookupType address_family);

  // Leaks memory if HasOutstandingRequests() is true.
  virtual ~DnsLookup();

  // Start asynchronous DNS lookup.
  //
  // Call once only.
  //
  // Eventually one of two events will be Emit()ed:
  // - kStateSuccess: DNS lookups succeeded. The result is available from the
  //   Sockaddr(), SockaddrLen() and IPAddress() methods.
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

  // The following methods are only valid when kStateSuccess is Emit()ed:

  // Returns the sockaddr struct of the successful lookup. The port is as
  // specified in the constructor.
  const sockaddr* Sockaddr() const;

  // Returns the length of the sockaddr struct.
  socklen_t SockaddrLen() const;

  // Returns a text representation of the IP address.
  string IPAddress() const;

  // Methods for choosing the LookupType.

  // Returns a short string describing |lookup_type|, suitable for displaying
  // to the user.
  static string LookupTypeName(const LookupType& address_family);

  // Returns the next LookupType in the list after |lookup_type|.
  //
  // For the last LookupType in the list, the first LookupType is
  // returned. This enables a CLI control to easily and obliviously cycle
  // through the possible options continuously.
  static LookupType NextLookupType(
      const LookupType& lookup_type);

  // In the kStateFail state returns a short string describing the error.
  string ErrorMessage() const;

 private:
  NO_COPY_AND_ASSIGN(DnsLookup)

  const string hostname_;
  char* port_str_;
  const enum LookupType lookup_type_;

  struct gaicb* request_ptrs[2];
  struct gaicb requests[2];
  struct addrinfo config[2];
  int request_count_;
  int result_index_;


  void SetState(State state, bool emit_event = false);
  State state_;

  bool HasBuggyGlibc() const;

  string error_message_;
};
}  // namespace x509ls

#endif  // X509LS_NET_DNS_LOOKUP_H_

