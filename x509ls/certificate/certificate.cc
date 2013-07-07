// X509LS
// Copyright 2013 Tom Harwood

#include "x509ls/certificate/certificate.h"

#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>

#include <sstream>

#include "x509ls/base/openssl/bio_translator.h"

using std::stringstream;

namespace x509ls {
Certificate::Certificate(const X509& x509,
    bool is_in_trust_store,
    bool is_in_peer_chain,
    bool is_in_validation_path)
  :
    x509_(X509_dup(const_cast<X509*>(&x509))),
    is_in_trust_store_(is_in_trust_store),
    is_in_peer_chain_(is_in_peer_chain),
    is_in_validation_path_(is_in_validation_path) {
  const size_t kMaxSubjectLength = 1024;
  char subject[kMaxSubjectLength];
  X509_NAME_oneline(X509_get_subject_name(x509_.Get()),
      subject, sizeof subject);
  subject_ = subject;

  X509_NAME* name = X509_get_subject_name(x509_.Get());

  // OID for Common Name.
  ASN1_OBJECT* obj = OBJ_txt2obj("2.5.4.3", 0);

  // Build a one line description of all Common Names.
  int pos = -1;
  while ((pos = X509_NAME_get_index_by_OBJ(name, obj, pos)) != -1) {
    X509_NAME_ENTRY* name_entry = X509_NAME_get_entry(name, pos);
    ASN1_STRING* asn1_string = X509_NAME_ENTRY_get_data(name_entry);

    unsigned char* final_utf8_string;
    int length = ASN1_STRING_to_UTF8(&final_utf8_string, asn1_string);

    if (!common_names_.empty()) {
      common_names_.append("/");
    }

    common_names_.append("CN=");

    common_names_.append(string(
          reinterpret_cast<char*>(final_utf8_string), length));

    OPENSSL_free(final_utf8_string);
  }
  ASN1_OBJECT_free(obj);

  // Text description.
  BioTranslator x509_text_bio;
  X509_print_ex(x509_text_bio.Get(), x509_.Get(), 0, 0);
  text_description_ = x509_text_bio.ToString();
  text_description_.append(AsPEM());
}

Certificate::~Certificate() {
}

string Certificate::Subject() const {
  return subject_;
}

string Certificate::CommonNames() const {
  return common_names_;
}

bool Certificate::IsSelfSigned() const {
  return X509_check_issued(x509_.Get(), x509_.Get()) == X509_V_OK;
}

bool Certificate::IsInTrustStore() const {
  return is_in_trust_store_;
}

bool Certificate::IsInPeerChain() const {
  return is_in_peer_chain_;
}

bool Certificate::IsInValidationPath() const {
  return is_in_validation_path_;
}

string Certificate::NotAfterDate() const {
  const ASN1_TIME* not_after = X509_get_notAfter(x509_.Get());
  stringstream date;

  const unsigned char* data = not_after->data;
  const int length = not_after->length;

  if (not_after->type == V_ASN1_UTCTIME) {
    if (length < 6 || !IsNumberString(data, 6)) {
      goto err;
    }
    int yyyy = (10 * (data[0] - '0')) + (data[1] - '0');
    yyyy += yyyy >= 50 ? 1900 : 2000;

    date << yyyy;
    date << "-";
    date << data[2] << data[3];
    date << "-";
    date << data[4] << data[5];
  } else if (not_after->type == V_ASN1_GENERALIZEDTIME) {
    if (length < 8 || !IsNumberString(data, 8)) {
      goto err;
    }
    date << data[0] << data[1] << data[2] << data[3];
    date << "-";
    date << data[4] << data[5];
    date << "-";
    date << data[6] << data[7];
  }

 err:
  if (date.str().size() == 0) {
    date << "????" << "-" << "??" << "-" << "??";
  }

  return date.str();
}

// static
bool Certificate::IsNumberString(const unsigned char* start, int length) {
  for (int i = 0; i < length; i++) {
    if (*(start + i) < '0' || *(start + i) > '9') {
      return false;
    }
  }

  return true;
}

string Certificate::TextDescription() const {
  return text_description_;
}

string Certificate::AsPEM() const {
  BioTranslator pem_bio;
  PEM_write_bio_X509(pem_bio.Get(), x509_.Get());

  return pem_bio.ToString();
}

}  // namespace x509ls

