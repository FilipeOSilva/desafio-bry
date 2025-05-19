#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/cms.h>
#include <openssl/x509.h>
#include <openssl/bio.h>
#include <openssl/asn1.h>
#include <openssl/objects.h>

#include <fstream>
#include <vector>
#include <iostream>
#include <iomanip>

#include "Constants.h"
#include "DocSignInfo.h"
#include "RSAKey.h"

void DocSignInfo::handleErrors(const std::string& msg) {
    std::cerr << msg << std::endl;
    ERR_print_errors_fp(stderr);
    throw std::runtime_error(msg);
}

std::string DocSignInfo::sha512(const std::string& input) {
    unsigned char hash[SHA512_DIGEST_LENGTH];
    SHA512(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), hash);
    std::stringstream s512;
    for (int i = 0; i < SHA512_DIGEST_LENGTH; ++i) {
        s512 << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return s512.str();
}

const unsigned char* DocSignInfo::getDoc() const {
    return reinterpret_cast<const unsigned char*>(doc.data());
}

size_t DocSignInfo::getDocSize() const {
    return doc.size();
}

const unsigned char* DocSignInfo::getSignature() const {
    return reinterpret_cast<const unsigned char*>(signature.data());
}

size_t DocSignInfo::getSignatureSize() const {
    return signature.size();
}

void DocSignInfo::setStatusCert(bool status) {
    statusCert = status;
}

void DocSignInfo::verify() {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    RSAKey keyRSA;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        handleErrors("Context Error.");
    }

    if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha512(), nullptr, keyRSA.getKeyPublic()) <= 0) {
        handleErrors("Context init verify error.");
    }

    if (EVP_DigestVerifyUpdate(ctx, getDoc(), getDocSize()) <= 0) {
        handleErrors("Context prepar verify error.");
    }

    int result = EVP_DigestVerifyFinal(ctx, getSignature(), getSignatureSize());
    if (result == 1) {
        setStatusCert(true);
    } else {
        setStatusCert(false);
    }

    EVP_MD_CTX_free(ctx);
}

void DocSignInfo::getCMSInfo() {
    BIO* cmsBio = BIO_new_mem_buf(getDoc(), getDocSize());

    if (!cmsBio) {
        handleErrors("Error to create CMS.");
    }

    CMS_ContentInfo* cms = SMIME_read_CMS(cmsBio, nullptr);

    if (!cms) {
        BIO_free(cmsBio);
        handleErrors("Parse CMS error.");
    }

    // BIO* out = BIO_new_fp(stdout, BIO_NOCLOSE);
    // CMS_ContentInfo_print_ctx(out, cms, 0, nullptr);

    STACK_OF(X509)* certs = CMS_get1_certs(cms);
    STACK_OF(CMS_SignerInfo)* signers = CMS_get0_SignerInfos(cms);

    if (!signers || sk_CMS_SignerInfo_num(signers) == 0) {
        handleErrors("Signate not found!");
    }

    CMS_SignerInfo* si = sk_CMS_SignerInfo_value(signers, 0);

    X509_NAME* issuer = nullptr;
    ASN1_INTEGER* serial = nullptr;

    if (!CMS_SignerInfo_get0_signer_id(si, nullptr, &issuer, &serial)) {
        handleErrors("Signater ID not found");
    }

    X509* signerCert = nullptr;
    for (int i = 0; i < sk_X509_num(certs); ++i) {
        X509* cert = sk_X509_value(certs, i);
        if (X509_NAME_cmp(issuer, X509_get_issuer_name(cert)) == 0 &&
            ASN1_INTEGER_cmp(serial, X509_get_serialNumber(cert)) == 0) {
            signerCert = cert;
            break;
       }
    }

    if (!signerCert) {
        handleErrors("Signater certify not found.");
    }

    X509_NAME* subj = X509_get_subject_name(signerCert);
    char cn[256];
    int len_x509 = X509_NAME_get_text_by_NID(subj, NID_commonName, cn, sizeof(cn));
    CN = (len_x509 >= 0) ? std::string(cn, len_x509) : "";

    int attrCount = CMS_signed_get_attr_count(si);
    ASN1_OBJECT* signingTimeOid = OBJ_nid2obj(NID_pkcs9_signingTime);

    for (int i = 0; i < attrCount; ++i) {
        X509_ATTRIBUTE* attr = CMS_signed_get_attr(si, i);
        ASN1_OBJECT* obj = X509_ATTRIBUTE_get0_object(attr);

        char objTxt[128];
        OBJ_obj2txt(objTxt, sizeof(objTxt), obj, 1);

        if (OBJ_cmp(obj, signingTimeOid) == 0) {
            ASN1_TYPE* val = X509_ATTRIBUTE_get0_type(attr, 0);
            if (val && val->type == V_ASN1_UTCTIME) {
                ASN1_UTCTIME* utc = val->value.utctime;
                char buffer[32];
                ASN1_TIME_to_tm(utc, nullptr);

                int len_utc = utc->length;
                memcpy(buffer, utc->data, len_utc);
                buffer[len_utc] = '\0';
                signingTime = (len_utc >= 0) ? std::string(buffer, len_utc) : "";
            }
        }
    }

    X509_ALGOR* digistInfo = nullptr;
    CMS_SignerInfo_get0_algs(si, nullptr, nullptr, &digistInfo, nullptr);

    if (digistInfo) {
        int nid = OBJ_obj2nid(digistInfo->algorithm);
        const char* digistAlgorithmCMS = OBJ_nid2ln(nid);
        if (digistAlgorithmCMS) {
            digestAlgorithm = digistAlgorithmCMS;
        }
    }
}

void DocSignInfo::getSignFile() {

    std::string nameFile = sha512(doc);
    std::ifstream file(Constants::SIGNATUREPATH+nameFile+".p7s", std::ios::binary);
    if (!file) {
        handleErrors("Sign not found, doc not valid!");
    }
    signature = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
}

DocSignInfo::DocSignInfo(const std::string& input)
    : doc(input) {
    getSignFile();
    verify();
    getCMSInfo();
}

bool DocSignInfo::getStatusCert() {
    return statusCert;
}

std::string DocSignInfo::getCN() {
    return CN;
}

std::string DocSignInfo::getSigningTime() {
    return signingTime;
}

std::string DocSignInfo::getEncapContentInfoHEXA() {
    return encapContentInfoHEXA;
}

std::string DocSignInfo::getDigestAlgorithm() {
    return digestAlgorithm;
}
