#include "DocToSignature.h"
#include "RSAKey.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>

void DocToSignature::handleErrors(const std::string& msg) {
    std::cerr << msg << std::endl;
    ERR_print_errors_fp(stderr);
    throw std::runtime_error(msg);
}

std::string DocToSignature::sha512(const std::string& input) {
    unsigned char hash[SHA512_DIGEST_LENGTH];
    SHA512(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), hash);
    std::stringstream s512;
    for (int i = 0; i < SHA512_DIGEST_LENGTH; ++i) {
        s512 << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return s512.str();
}

const unsigned char* DocToSignature::getDocPKCS12() const {
    return reinterpret_cast<const unsigned char*>(docPKCS12.data());
}

size_t DocToSignature::getDocPKCS12Size() const {
    return docPKCS12.size();
}

const char* DocToSignature::getKeyPKCS12() const {
    return keyPKCS12.c_str();
}

void DocToSignature::getInfoPKCS12() {
    const unsigned char* buffer = getDocPKCS12();
    PKCS12* p12 = d2i_PKCS12(nullptr, &buffer, getDocPKCS12Size());
    if (!PKCS12_parse(p12, getKeyPKCS12(), &pkey, &cert, nullptr)) {
        handleErrors("Key not valid!");
    }
    PKCS12_free(p12);
}

void DocToSignature::buildCMS() {
    CMS_ContentInfo* cms = nullptr;
    BIO* input_bio = BIO_new_mem_buf(docIn.c_str(), docIn.size());

    if (!input_bio) {
        handleErrors("File cms Not Init.");
    }

    cms = CMS_sign(cert, pkey, nullptr, input_bio, CMS_STREAM | CMS_BINARY | CMS_DETACHED);
    BIO* output_bio = BIO_new(BIO_s_mem());

    if (!output_bio) {
        handleErrors("File cms out Not Init.");
    }

    if (!SMIME_write_CMS(output_bio, cms, input_bio, CMS_STREAM | CMS_BINARY | CMS_DETACHED)) {
        BIO_free(output_bio);
        handleErrors("Not write cms file.");
    }

    BUF_MEM* bufferPtr = nullptr;
    BIO_get_mem_ptr(output_bio, &bufferPtr);
    docCMS.assign(bufferPtr->data, bufferPtr->length);

    BIO_free_all(input_bio);
    BIO_free_all(output_bio);
    CMS_ContentInfo_free(cms);
}

void DocToSignature::signDoc() {
    OpenSSL_add_all_algorithms();

    RSAKey keyRSA;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();

    if (EVP_DigestSignInit(ctx, nullptr, EVP_sha512(), nullptr, keyRSA.getKeyPriv()) <= 0) {
        handleErrors("Context encrypt sign error.");
    }

    if (EVP_DigestSignUpdate(ctx, docCMS.c_str(), docCMS.size()) <= 0) {
        handleErrors("Context prepar error.");
    }

    size_t sigLen = 0;
    if (EVP_DigestSignFinal(ctx, nullptr, &sigLen) <= 0) {
        handleErrors("Sign not possible.");
    }

    std::vector<unsigned char> signature(sigLen);
    if (EVP_DigestSignFinal(ctx, signature.data(), &sigLen) <= 0) {
        handleErrors("Sign not possible.");
    }

    std::string nameFile = sha512(docCMS);

    if (!std::filesystem::exists(Constants::SIGNATUREPATH)) {
        std::filesystem::create_directories(std::filesystem::path(Constants::SIGNATUREPATH).parent_path());
    }

    std::ofstream out(Constants::SIGNATUREPATH+nameFile+".p7s", std::ios::binary);
    out.write(reinterpret_cast<const char*>(signature.data()), sigLen);
    out.close();

    EVP_MD_CTX_free(ctx);
}

DocToSignature::DocToSignature(const std::string& base64_input, const std::string& input_key, const std::string& doc)
    : keyPKCS12(input_key), docIn(doc), docPKCS12(base64_input) {
    getInfoPKCS12();
    buildCMS();
    signDoc();
}

std::string DocToSignature::getDocSignBase64() {
    BIO* mem = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_push(b64, mem);

    BIO_write(b64, docCMS.c_str(), docCMS.size());
    BIO_flush(b64);

    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(mem, &bufferPtr);

    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(b64);

    return result;
}
