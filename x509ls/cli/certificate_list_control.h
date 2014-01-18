// X509LS
// Copyright 2013 Tom Harwood

#ifndef X509LS_CLI_CERTIFICATE_LIST_CONTROL_H_
#define X509LS_CLI_CERTIFICATE_LIST_CONTROL_H_


#include "x509ls/base/types.h"
#include "x509ls/cli/base/colours.h"
#include "x509ls/cli/base/list_control.h"

namespace x509ls {
class Certificate;
class CertificateList;
// CLI control: A scrollable list of SSL certificates.
//
// Displays an ordered list of SSL certificates. Each certificate row contains:
// - The certificate name (either the Subject Common Name, or the full Subject
//   if no Subject CN is present).
// - The certificate notAfter date (expiry date), in UTC.
// - A number to assist navigation of the list.
// - Single character flags, displayed if true:
//   - s: certificate is self-signed.
//   - t: certificate is present in the trust store.
//   - v: certificate is present in the validation path formed by OpenSSL.
//   - c: certificate is present in the server's chain.
//
// There are two display styles, determined by the type of list being displayed:
// - Validation path style, e.g.:
// stv.  1 + CN=DigiCert High Assurance EV Root CA                    2031-11-10
// ..vc  2  + CN=DigiCert High Assurance EV CA-1                      2021-11-10
// ..vc  3   + CN=www.digicert.com                                    2014-05-17
//
// - Peer chain style, e.g.:
// ..vc  1 CN=www.digicert.com                                        2014-05-17
// ..vc  2 CN=DigiCert High Assurance EV CA-1                         2021-11-10
//
// The 't' flag is not supported with pre-v1.0.0 versions of OpenSSL. It is not
// displayed in this case.
class CertificateListControl : public ListControl {
 public:
  // Display style types.
  enum ListType {
    kTypePeerChain,
    kTypeValidationPath
  };

  // Construct a CertificateListControl with |parent|, of |list_type| and
  // optional list of certificates to display |model|.
  //
  // |model| must exist for the lifetime of the CertificateListControl, or
  // otherwise until a different model is installed using SetModel().
  CertificateListControl(CliControl* parent,
      ListType list_type,
      const CertificateList* model = NULL);
  virtual ~CertificateListControl();

  // Display the list of certificates in |model|. |model| may be NULL.
  //
  // The screen is repainted as necessary to display |model|'s certificates.
  // See the constructor description for |model|'s lifetime requirements.
  void SetModel(const CertificateList* model);

  // Returns the current certificate list.
  const CertificateList* Model() const;

  // Return the Certificate currently selected, or NULL if none is selected.
  const Certificate* CurrentCertificate() const;

 protected:
  // Print row for the certificate at offset |index| into the window row |row|.
  // |selected| indicates the row is selected and should be appropriately
  // highlighted.
  virtual void PaintLine(unsigned int index, unsigned int row, bool selected);

 private:
  NO_COPY_AND_ASSIGN(CertificateListControl)

  const CertificateList* model_;
  const ListType list_type_;

  // Print a single character flag in |colour_type| style.
  //
  // If |flag| print |symbol|, otherwise print '.'.
  void PrintFlag(bool flag, char symbol,
      Colours::ColourType colour_type);

  // Returns true if |c| is an unprintable character (i.e. a control character).
  static bool IsUnprintableChar(char c);
};
}  // namespace x509ls

#endif  // X509LS_CLI_CERTIFICATE_LIST_CONTROL_H_

