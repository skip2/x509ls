// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/net/dns_lookup.h"

#include <assert.h>
#include <ctype.h>
#include <gnu/libc-version.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>

using std::stringstream;

namespace x509ls {
DnsLookup::DnsLookup(BaseObject* parent, const string& node,
      const string& service, LookupType lookup_type)
  :
    BaseObject(parent),
    node_(node),
    service_(service),
    lookup_type_(lookup_type),
    request_count_(0),
    result_index_(-1),
    state_(kStateStart) {
}

// virtual
DnsLookup::~DnsLookup() {
  for (int i = 0; i < request_count_; ++i) {
    int state = gai_cancel(&requests[i]);
    if (state == EAI_CANCELED || state == EAI_ALLDONE) {
      freeaddrinfo(requests[i].ar_result);
    }
  }
}

void DnsLookup::Start() {
  assert(request_count_ == 0);
  assert(GetState() == kStateStart);

  SetState(kStateInProgress);

  if (HasBuggyGlibc()) {
    SetState(kStateFail, true);
    error_message_ = "A bug your glibc version prevents async DNS lookups,"
      " see the README, sorry!";
    return;
  }

  int ipv4_index = -1;
  int ipv6_index = -1;
  request_count_ = 0;

  switch (lookup_type_) {
  case kLookupTypeIPv4:
    ipv4_index = 0;
    break;
  case kLookupTypeIPv6:
    ipv6_index = 0;
    break;
  case kLookupTypeIPv4then6:
    ipv4_index = 0;
    ipv6_index = 1;
    break;
  case kLookupTypeIPv6then4:
    ipv4_index = 1;
    ipv6_index = 0;
    break;
  default:
    break;
  }

  for (int i = 0; i < 2; ++i) {
    config[i].ai_flags = 0;
    config[i].ai_socktype = SOCK_STREAM;
    config[i].ai_protocol = IPPROTO_TCP;
    config[i].ai_addrlen = 0;
    config[i].ai_addr = NULL;
    config[i].ai_canonname = NULL;
    config[i].ai_next = NULL;
  }
  config[0].ai_family = AF_INET;
  config[1].ai_family = AF_INET6;

  if (ipv4_index != -1) {
    request_count_++;
    requests[ipv4_index].ar_name = node_.c_str();
    requests[ipv4_index].ar_service = service_.c_str();
    requests[ipv4_index].ar_request = &config[0];
    requests[ipv4_index].ar_result = NULL;
  }

  if (ipv6_index != -1) {
    request_count_++;
    requests[ipv6_index].ar_name = node_.c_str();
    requests[ipv6_index].ar_service = service_.c_str();
    requests[ipv6_index].ar_request = &config[1];
    requests[ipv6_index].ar_result = NULL;
  }

  request_ptrs[0] = &requests[0];
  request_ptrs[1] = &requests[1];

  getaddrinfo_a(GAI_NOWAIT, request_ptrs, request_count_, NULL);

  EnablePoll();
}

// virtual
void DnsLookup::OnPoll() {
  for (int i = 0; i < request_count_; ++i) {
    int gai_state = gai_error(request_ptrs[i]);
    switch (gai_state) {
    case EAI_INPROGRESS:
      // Wait for finish.
      return;
    case 0:
      // Success, return result.
      result_index_ = i;
      SetState(kStateSuccess, true);
      DisablePoll();
      return;
    default:
      // Error? If any more requests, use them instead, otherwise fail if all
      // the requests resulted in errors.
      if (i == request_count_ - 1) {
        // Last request (thus all requests) have errors?
        SetState(kStateFail, true);
        error_message_ = "Name/service lookup failed.";
        DisablePoll();
        return;
      }
    }
  }
}

// static
string DnsLookup::LookupTypeName(
    const DnsLookup::LookupType& lookup_type) {
  string result;
  switch (lookup_type) {
  case kLookupTypeIPv4:
    result = "IPv4";
    break;
  case kLookupTypeIPv6:
    result = "IPv6";
    break;
  case kLookupTypeIPv4then6:
    result = "IPv4,6";
    break;
  case kLookupTypeIPv6then4:
    result = "IPv6,4";
    break;
  default:
    break;
  }

  return result;
}

// static
DnsLookup::LookupType DnsLookup::NextLookupType(
    const DnsLookup::LookupType& lookup_type) {
  int new_lookup_type = static_cast<int>(lookup_type) + 1;
  if (new_lookup_type > kLookupTypeIPv6then4) {
    new_lookup_type = static_cast<int>(kLookupTypeIPv4);
  }

  return static_cast<LookupType>(new_lookup_type);
}

const sockaddr* DnsLookup::Sockaddr() const {
  assert(result_index_ != -1);
  return requests[result_index_].ar_result->ai_addr;
}

socklen_t DnsLookup::SockaddrLen() const {
  assert(result_index_ != -1);
  return requests[result_index_].ar_result->ai_addrlen;
}

string DnsLookup::IPAddressAndPort() const {
  if (result_index_ == -1) {
    return "";
  }

  char host[NI_MAXHOST];
  char port[NI_MAXSERV];
  getnameinfo(Sockaddr(), SockaddrLen(), host, sizeof(host),
      port, sizeof(port), NI_NUMERICHOST | NI_NUMERICSERV);

  stringstream result;
  if (Sockaddr()->sa_family == AF_INET6) {
    result << "[";
    result << host;
    result << "]:";
    result << port;
  } else {
    result << host;
    result << ":";
    result << port;
  }

  return result.str();
}

void DnsLookup::Cancel() {
  for (int i = 0; i < request_count_; ++i) {
    gai_cancel(&requests[i]);
  }
}

bool DnsLookup::HasOutstandingRequests() const {
  bool has_outstanding_requests = false;
  for (int i = 0; i < request_count_; ++i) {
    if (gai_error(const_cast<struct gaicb*>(&requests[i])) == EAI_INPROGRESS) {
      has_outstanding_requests = true;
      break;
    }
  }

  return has_outstanding_requests;
}

void DnsLookup::SetState(State state, bool emit_event) {
  state_ = state;

  if (emit_event) {
    Emit(state_);
  }
}

string DnsLookup::ErrorMessage() const {
  return error_message_;
}

bool DnsLookup::HasBuggyGlibc() const {
  const char* glibc_version = gnu_get_libc_version();
  const string bad_versions[] = {
    "2.5",
    "2.6",
    "2.7",
    "2.8",
    ""
  };

  bool is_buggy = false;
  for (const string* str = &bad_versions[0]; !str->empty(); ++str) {
    if (strcmp(glibc_version, str->c_str()) == 0) {
      is_buggy = true;
      break;
    } else if (strlen(glibc_version) > str->size() &&
        strncmp(glibc_version, str->c_str(), str->size()) == 0 &&
        !isdigit(glibc_version[str->size()])) {
      is_buggy = true;
      break;
    }
  }

  return is_buggy;
}

DnsLookup::State DnsLookup::GetState() const {
  return state_;
}
}  // namespace x509ls

