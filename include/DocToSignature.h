#ifndef DOC_TO_SIGNATURE_H
#define DOC_TO_SIGNATURE_H

#include <openssl/pkcs12.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/cms.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#include "RSAKey.h"
#include "Constants.h"

class DocToSignature {
private:
    std::string keyPKCS12;
    std::string docIn;
    std::string docPKCS12;
    std::string docCMS;

    EVP_PKEY* pkey = nullptr;
    X509* cert = nullptr;

    void handleErrors(const std::string& msg);
    void decodeBase64(const std::string& b64_input);
    std::string sha512(const std::string& input);
    const unsigned char* getDocPKCS12() const;
    size_t getDocPKCS12Size() const;
    const char* getKeyPKCS12() const;
    void getInfoPKCS12();
    void buildCMS();
    void signDoc();

public:
    DocToSignature(const std::string& base64_input, const std::string& input_str, const std::string& doc);
    std::string getDocSignBase64();

};

#endif // DOC_TO_SIGNATURE_H