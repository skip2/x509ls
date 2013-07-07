// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/cli/certificate_list_control.h"

#include <assert.h>
#include <ncurses.h>
#include <stddef.h>

#include <algorithm>
#include <sstream>
#include <string>

#include "x509ls/base/openssl/openssl_environment.h"
#include "x509ls/certificate/certificate_list.h"
#include "x509ls/cli/base/list_model.h"

using std::max;
using std::string;

namespace x509ls {
CertificateListControl::CertificateListControl(CliControl* parent,
    ListType list_type,
    const CertificateList* model)
  :
  ListControl(parent, model),
  model_(model),
  list_type_(list_type) {
}

// virtual
CertificateListControl::~CertificateListControl() {
}

// virtual
void CertificateListControl::PaintLine(unsigned int index, unsigned int row,
    bool selected) {
  const Certificate& cert = (*model_)[index];

  WINDOW* window = Window();

  if (selected) {
    wattron(window, Colours::Get(Colours::kColourHighlighted));
  }

  wmove(window, row, 0);
  wprintw(window, string(Cols(), ' ').c_str());
  wmove(window, row, 0);

  // Displays certificates in list or hierarchy format, based on |list_type_|.
  // stpc   1 GlobalSign Root CA                                      2024-01-02
  // stpc   1 + GlobalSign Root CA                                    2024-01-02
#ifdef X509LS_OLD_OPENSSL_NO_TRUST_STORE_LOOKUP
  // OpenSSL <v1 doesn't have the X509_STORE_get1_certs function needed for the
  // 't' (in trust store) flag. Omit showing it.
  const int kFlagsSize = 3;
#else
  const int kFlagsSize = 4;
#endif

  bool show_expiry = false;
  int cols_for_common_name = Cols();
  cols_for_common_name -= kFlagsSize + 1;  // Flags column + sp char.
  cols_for_common_name -= 2 + 1;  // Row number column + sp char.

  // Verification paths are shown in a hierarchical representation.
  // 01234
  // + xxx
  //  + xxx
  //   + xxx
  string hierarchy_diagram;
  if (list_type_ == kTypeVerificationPath) {
    cols_for_common_name -= 2 + index;
    hierarchy_diagram = string(index, ' ');
    hierarchy_diagram.append("+ ");
  }

  // Enough space to show the expiry column?
  const int kExpiryColSize = 11;
  if (cols_for_common_name > 2 * kExpiryColSize + 1) {
    show_expiry = true;
    cols_for_common_name -= kExpiryColSize + 1;  // Expiry date & sp char.
  }

  string common_name = cert.CommonNames();
  if (common_name.empty()) {
    common_name = cert.Subject();
  }

  if (static_cast<int>(common_name.size()) > cols_for_common_name) {
    common_name.erase(max(0, cols_for_common_name - 3));
    common_name.append("...");
  }

  PrintFlag(cert.IsSelfSigned(), 's',
      selected ? Colours::kColourRedHighlighted : Colours::kColourRed);

#ifndef X509LS_OLD_OPENSSL_NO_TRUST_STORE_LOOKUP
  PrintFlag(cert.IsInTrustStore(), 't',
      selected ? Colours::kColourYellowHighlighted : Colours::kColourYellow);
#endif

  PrintFlag(cert.IsInValidationPath(), 'v',
      selected ? Colours::kColourGreenHighlighted : Colours::kColourGreen);

  PrintFlag(cert.IsInPeerChain(), 'c',
      selected ? Colours::kColourPurpleHighlighted : Colours::kColourPurple);

  wattrset(Window(), Colours::Get(
      selected ? Colours::kColourHighlighted : Colours::kColourDefault));

  wattron(Window(), A_BOLD);

  wprintw(window, " %2d ",
      index + 1);  // Convert from zero-indexed to one-indexed for humans.

  if (!hierarchy_diagram.empty()) {
    wprintw(window, "%s", hierarchy_diagram.c_str());
  }

  wattroff(Window(), A_BOLD);

  wprintw(window, "%s",
      common_name.c_str());

  if (show_expiry) {
    wmove(window, row, Cols() - kExpiryColSize);
    wattron(Window(), A_BOLD);
    wprintw(window, cert.NotAfterDate().c_str());
    wattroff(Window(), A_BOLD);
  }

  if (selected) {
    wattroff(window, Colours::Get(Colours::kColourHighlighted));
  }
}

void CertificateListControl::PrintFlag(bool flag, char symbol,
    Colours::ColourType colour_type) {
  wattron(Window(), Colours::Get(colour_type));
  wprintw(Window(), "%c", flag ? symbol : '.');
}

// virtual
void CertificateListControl::SetModel(const CertificateList* model) {
  model_ = model;
  ListControl::SetModel(model);
}

const Certificate* CertificateListControl::CurrentCertificate() const {
  const int index = SelectedIndex();
  if (index == -1) {
    return NULL;
  }

  return &(*model_)[index];
}
}  // namespace x509ls

