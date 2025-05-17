// SignatureVerifier.h
#ifndef DOCSIGHINFO_H
#define DOCSIGHINFO_H

#include <string>

class DocSignInfo {
private:
    std::vector<unsigned char> doc;
    std::vector<unsigned char> signature;
    bool statusCert = false;

    void getSignFile();
    size_t getSignatureSize() const;
    const unsigned char* getSignature() const;
    size_t getDocSize() const;
    const unsigned char* getDoc() const;
    void handleErrors(const std::string& msg);
    void decodeBase64(const std::string& b64_input);
    void verify();
    void setStatusCert(bool status);
    void getCMSInfo();

public:
    DocSignInfo(const std::string& base64_input);
    bool getStatusCert();
    void getCN();
    void signingTime();
    void encapContentInfoHEXA();
    void digestAlgorithm();
};

#endif
