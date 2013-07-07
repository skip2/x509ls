// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_CERTIFICATE_VIEW_LAYOUT_H_
#define X509LS_CLI_CERTIFICATE_VIEW_LAYOUT_H_

#include <string>

#include "x509ls/base/types.h"
#include "x509ls/certificate/certificate.h"
#include "x509ls/cli/base/cli_control.h"

using std::string;

namespace x509ls {
class CliApplication;
class CommandLine;
class MenuBar;
class StatusBar;
class TextControl;

// Screen layout for displaying a single scrollable certificate.
//
// The screen layout consists of a menu bar, scrollable text representation of
// the certificate (the majority of space), spacer rows, and a command line row
// for text entry.
//
// There are two menu options:
//  - index: Close layout, go back to previous layout, i.e. certificate list.
//  - save: Save certificate in PEM format.
class CertificateViewLayout : public CliControl {
 public:
  // Construct a CertificateViewLayout. |certificate| should exist for the
  // lifetime of the object.
  CertificateViewLayout(CliApplication* application,
      const Certificate& certificate);
  virtual ~CertificateViewLayout();

 protected:
  virtual bool KeyPressEvent(int keypress);
  virtual void OnEvent(const BaseObject* source, int event_code);

 private:
  NO_COPY_AND_ASSIGN(CertificateViewLayout)

  const Certificate& certificate_;

  MenuBar* menu_bar_;
  TextControl* text_control_;
  StatusBar* status_bar_;
  CommandLine* command_line_;

  static const char* kMenuText;

  // Display the save certificate (as filename) prompt.
  void StartSaveCertificate();

  // Save the certificate to |filename| in PEM format, and set |result_message|
  // to a short string indicating success or failure. Returns true iif the
  // certificate was saved successfully.
  bool SaveCertificate(const string& filename, string* result_message) const;
};
}  // namespace x509ls

#endif  // X509LS_CLI_CERTIFICATE_VIEW_LAYOUT_H_

