// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/x509ls.h"

#include <getopt.h>
#include <stdio.h>

#include <string>

#include "x509ls/cli/certificate_list_layout.h"

using std::string;

namespace x509ls {
X509LS::X509LS()
  :
    CliApplication() {
}

// virtual
X509LS::~X509LS() {
}

bool X509LS::Init(int argc, char** argv) {
  static const struct option options[] = {
    {"capath", required_argument, NULL, 'p'},
    {"cafile", required_argument, NULL, 'f'},
    {0, 0, 0, 0}
  };

  bool success = true;
  bool custom_trust_store = false;
  int getopt_flag;
  string error_message;
  do {
    int option_index;
    getopt_flag = getopt_long(argc, argv, "", options, &option_index);

    switch (getopt_flag) {
    case 'p':
      if (!trust_store_.AddCAPath(optarg, &error_message)) {
        success = false;
        fprintf(stderr, "%s", error_message.c_str());
      }
      custom_trust_store = true;
      break;
    case 'f':
      if (!trust_store_.AddCAFile(optarg, &error_message)) {
        success = false;
        fprintf(stderr, "%s", error_message.c_str());
      }
      custom_trust_store = true;
      break;
    case -1:
      // No more options to parse.
      break;
    case '?':
    default:
      success = false;
    }
  } while (getopt_flag != -1);

  if (!custom_trust_store) {
    trust_store_.AddSystemCAPath();
  }

  if (optind == argc - 1) {
    host_port_ = argv[optind];
  } else if (optind < argc - 1) {
    fprintf(stderr,
        "Unexpected arguments, expecting a host:port argument only.\n");
    success = false;
  }

  return success;
}

// virtual
void X509LS::RunEvent() {
  CertificateListLayout* app = new CertificateListLayout(this, &trust_store_);
  Show(app);  // Ownership of app transfered here.

  if (!host_port_.empty()) {
    app->GotoHost(host_port_);
  }
}
}  // namespace x509ls

