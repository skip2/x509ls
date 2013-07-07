// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/cli/certificate_view_layout.h"

#include <errno.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

#include "x509ls/cli/base/cli_application.h"
#include "x509ls/cli/base/command_line.h"
#include "x509ls/cli/base/text_control.h"
#include "x509ls/cli/menu_bar.h"
#include "x509ls/cli/status_bar.h"

namespace x509ls {

// static
const char* CertificateViewLayout::kMenuText =
  "q:index s:save";

CertificateViewLayout::CertificateViewLayout(CliApplication* application,
    const Certificate& certificate)
  :
    CliControl(application),
    certificate_(certificate),
    menu_bar_(new MenuBar(this, kMenuText)),
    text_control_(new TextControl(this, certificate_.TextDescription())),
    status_bar_(new StatusBar(this)),
    command_line_(new CommandLine(this)) {
  AddChild(menu_bar_);
  AddChild(text_control_);
  AddChild(status_bar_);
  AddChild(command_line_);
  SetFocusedChild(text_control_);
}

// virtual
CertificateViewLayout::~CertificateViewLayout() {
}

// virtual
bool CertificateViewLayout::KeyPressEvent(int keypress) {
  bool handled = false;
  switch (keypress) {
  case 'i':
  case 'q':
    handled = true;
    GetApplication()->Close(this);
    break;
  case 's':
    StartSaveCertificate();
    handled = true;
  }

  return handled;
}

void CertificateViewLayout::StartSaveCertificate() {
  SetFocusedChild(command_line_);
  Subscribe(command_line_, CommandLine::kEventInputAccepted);
  Subscribe(command_line_, CommandLine::kEventInputCancelled);
  command_line_->DisplayPrompt("Save as file: ", ".pem");
}

// virtual
void CertificateViewLayout::OnEvent(const BaseObject* source, int event_code) {
  if (source == command_line_) {
    if (event_code == CommandLine::kEventInputCancelled) {
      command_line_->Clear();
    } else if (event_code == CommandLine::kEventInputAccepted) {
      const string filename = command_line_->InputText();

      string result_message;
      SaveCertificate(filename, &result_message);
      command_line_->DisplayMessage(result_message);
    }

    SetFocusedChild(text_control_);
  }
}

bool CertificateViewLayout::SaveCertificate(
    const string& filename, string* result_message) const {
  const string pem_certificate = certificate_.AsPEM();

  FILE* file = fopen(filename.c_str(), "w");
  if (!file) {
    *result_message = "Error opening file: ";
    result_message->append(strerror(errno));
    return false;
  }

  size_t bytes_written = fwrite(pem_certificate.c_str(),
      sizeof(char),  // NOLINT(runtime/sizeof)
      pem_certificate.size(), file);

  bool success = true;

  if (bytes_written != pem_certificate.size()) {
    *result_message = "Error writing file: ";
    result_message->append(strerror(errno));
    success = false;
  } else {
    *result_message = "Certificate saved.";
  }

  fclose(file);

  return success;
}
}  // namespace x509ls

