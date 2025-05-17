#include "RSAKey.h"
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <filesystem>
#include <fstream>
#include <iostream>

void RSAKey::handleErrors(const std::string& msg) {
    std::cerr << msg << std::endl;
    ERR_print_errors_fp(stderr);
    throw std::runtime_error(msg);
}

void RSAKey::generateRSAKey(int bits) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        handleErrors("Error to create Context RSA.");
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        handleErrors("Error init key RSA.");
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0) {
        handleErrors("Error in len key RSA.");
    }

    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        handleErrors("Error to create key RSA.");
    }

    EVP_PKEY_CTX_free(ctx);
}

void RSAKey::saveKeys() {
    std::filesystem::create_directories(std::filesystem::path(privPath).parent_path());

    FILE* privFile = fopen(privPath.c_str(), "w");
    if (!privFile) {
        handleErrors("Error to open private key.");
    }

    if (!PEM_write_PrivateKey(privFile, pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
        handleErrors("Error to save private key.");
    }
    fclose(privFile);

    FILE* pubFile = fopen(pubPath.c_str(), "w");
    if (!pubFile) {
        handleErrors("Error to open public key.");
    }

    if (!PEM_write_PUBKEY(pubFile, pkey)) {
        handleErrors("Error to save public key.");
    }
    fclose(pubFile);
}

void RSAKey::loadKeys() {
    FILE* privFile = fopen(privPath.c_str(), "r");
    if (!privFile) {
        handleErrors("Error to open private key.");
    }

    privKey = PEM_read_PrivateKey(privFile, nullptr, nullptr, nullptr);
    if (!privKey) {
        handleErrors("Error to read private key.");
    }
    fclose(privFile);

    FILE* pubFile = fopen(pubPath.c_str(), "r");
    if (!pubFile) {
        handleErrors("Error to open public key.");
    }

    pubKey = PEM_read_PUBKEY(pubFile, nullptr, nullptr, nullptr);
    if (!pubKey) {
        handleErrors("Error to read public key.");
    }
    fclose(pubFile);
}

RSAKey::RSAKey() {
    if (std::filesystem::exists(privPath) && std::filesystem::exists(pubPath)) {
        loadKeys();
    } else {
        generateRSAKey(2048);
        saveKeys();
    }
}

EVP_PKEY* RSAKey::getKeyPublic() const {
    return pubKey;
}

EVP_PKEY* RSAKey::getKeyPriv() const {
    return privKey;
}
