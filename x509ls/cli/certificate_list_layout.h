// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_CERTIFICATE_LIST_LAYOUT_H_
#define X509LS_CLI_CERTIFICATE_LIST_LAYOUT_H_

#include <string>

#include "x509ls/base/types.h"
#include "x509ls/cli/base/cli_control.h"
#include "x509ls/net/dns_lookup.h"

using std::string;

namespace x509ls {
class CertificateListControl;
class ChainFetcher;
class CliApplication;
class CommandLine;
class MenuBar;
class StatusBar;
class TextControl;
class TrustStore;

// Main screen layout.
//
// Displays a list of certificates and a preview of the current certificate.
// Accepts text input to allow specifying a SSL server, and toggling of various
// options such as IPv4/6. Dispatches and owns network requests.
class CertificateListLayout : public CliControl {
 public:
  CertificateListLayout(CliApplication* application, TrustStore* trust_store);
  virtual ~CertificateListLayout();

  virtual void OnEvent(const BaseObject* source, int event_code);

 protected:
  virtual bool KeyPressEvent(int keypress);

 private:
  NO_COPY_AND_ASSIGN(CertificateListLayout)

  // The TrustStore to use for validating certificates.
  TrustStore* const trust_store_;

  // The menu text ("q:Quit"...).
  static const char* kMenuText;

  // ---------------------------------------------------------------------------
  // Screen layout.
  MenuBar* menu_bar_;
  CertificateListControl* list_controls_[2];  // One displayed at a time.
  StatusBar* top_status_bar_;
  TextControl* text_control_;
  StatusBar* bottom_status_bar_;
  CommandLine* command_line_;

  // Indexes for |list_controls_|.
  static const int kListControlIndexVerificationPath;
  static const int kListControlIndexPeerChain;

  // Index of the currently displayed |list_controls_|.
  int displayed_list_control_index_;

  // ---------------------------------------------------------------------------
  // Text input mode.
  enum TextInputType {
    kTextInputTypeGo,   // Text input of host in progress.
    kTextInputTypeNone  // No text input in progress.
  };
  enum TextInputType current_text_input_type_;

  void ShowGotoHostPrompt();
  void GotoHost(const string& hostname);

  // ---------------------------------------------------------------------------
  // Toggle options.
  enum DnsLookup::LookupType lookup_type_;
  size_t tls_method_index_;
  size_t tls_auth_type_index_;
  void UpdateStatusBarOptionsText();

  // ---------------------------------------------------------------------------
  // Stored user input, to enable reloading of a
  string user_input_hostname_;

  // ---------------------------------------------------------------------------
  // Current network worker.
  ChainFetcher* current_fetcher_;

  // ---------------------------------------------------------------------------
  // UI methods.
  void DisplayLoadingMessage();
  void DisplayConnectSuccessMessage();
  void DisplayConnectFailedMessage();

  string LocationText() const;

  void ToggleDisplayedListControl();
  void UpdateDisplayedCertificate();
  void ShowCertificateViewLayout();
};
}  // namespace x509ls

#endif  // X509LS_CLI_CERTIFICATE_LIST_LAYOUT_H_

