// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/net/dns_lookup.h"

#include <assert.h>
#include <ctype.h>
#include <gnu/libc-version.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>

namespace x509ls {
DnsLookup::DnsLookup(BaseObject* parent, const string& hostname,
      uint16 port, AddressFamily address_family)
  :
    BaseObject(parent),
    hostname_(hostname),
    port_(port),
    address_family_(address_family),
    request_count_(0),
    result_index_(-1),
    state_(kStateStart) {
  std::stringstream port_str;
  port_str << port_;

  port_str_ = strdup(port_str.str().c_str());
}

// virtual
DnsLookup::~DnsLookup() {
  bool still_need_port_str = false;

  for (int i = 0; i < request_count_; ++i) {
    int state = gai_cancel(&requests[i]);
    if (state == EAI_CANCELED || state == EAI_ALLDONE) {
      freeaddrinfo(requests[i].ar_result);
    } else {
      still_need_port_str = true;
    }
  }

  if (!still_need_port_str) {
    free(port_str_);
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

  switch (address_family_) {
  case kAddressFamilyIPv4:
    ipv4_index = 0;
    break;
  case kAddressFamilyIPv6:
    ipv6_index = 0;
    break;
  case kAddressFamilyIPv4then6:
    ipv4_index = 0;
    ipv6_index = 1;
    break;
  case kAddressFamilyIPv6then4:
    ipv4_index = 1;
    ipv6_index = 0;
    break;
  default:
    break;
  }

  for (int i = 0; i < 2; ++i) {
    config[i].ai_flags = AI_NUMERICSERV;
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
    requests[ipv4_index].ar_name = hostname_.c_str();
    requests[ipv4_index].ar_service = port_str_;
    requests[ipv4_index].ar_request = &config[0];
    requests[ipv4_index].ar_result = NULL;
  }

  if (ipv6_index != -1) {
    request_count_++;
    requests[ipv6_index].ar_name = hostname_.c_str();
    requests[ipv6_index].ar_service = port_str_;
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
        error_message_ = "DNS lookup failed.";
        DisablePoll();
        return;
      }
    }
  }
}

// static
string DnsLookup::AddressFamilyName(
    const DnsLookup::AddressFamily& address_family) {
  string result;
  switch (address_family) {
  case kAddressFamilyIPv4:
    result = "IPv4";
    break;
  case kAddressFamilyIPv6:
    result = "IPv6";
    break;
  case kAddressFamilyIPv4then6:
    result = "IPv4,6";
    break;
  case kAddressFamilyIPv6then4:
    result = "IPv6,4";
    break;
  default:
    break;
  }

  return result;
}

// static
DnsLookup::AddressFamily DnsLookup::NextAddressFamily(
    const DnsLookup::AddressFamily& address_family) {
  int new_address_family = static_cast<int>(address_family) + 1;
  if (new_address_family > kAddressFamilyIPv6then4) {
    new_address_family = static_cast<int>(kAddressFamilyIPv4);
  }

  return static_cast<AddressFamily>(new_address_family);
}

const sockaddr* DnsLookup::Sockaddr() const {
  assert(result_index_ != -1);
  return requests[result_index_].ar_result->ai_addr;
}

socklen_t DnsLookup::SockaddrLen() const {
  assert(result_index_ != -1);
  return requests[result_index_].ar_result->ai_addrlen;
}

string DnsLookup::IPAddress() const {
  if (result_index_ == -1) {
    return "";
  }

  char host[NI_MAXHOST];
  getnameinfo(Sockaddr(), SockaddrLen(), host, sizeof(host),
      NULL, 0, NI_NUMERICHOST);

  return host;
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

