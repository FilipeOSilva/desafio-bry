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
    exit(EXIT_FAILURE);
}

void RSAKey::generateRSAKey(int bits) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) handleErrors("Erro ao criar contexto para chave RSA.");

    if (EVP_PKEY_keygen_init(ctx) <= 0)
        handleErrors("Erro ao inicializar geração de chave RSA.");

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0)
        handleErrors("Erro ao configurar tamanho da chave RSA.");

    if (EVP_PKEY_keygen(ctx, &pkey) <= 0)
        handleErrors("Erro ao gerar a chave RSA.");

    EVP_PKEY_CTX_free(ctx);
}

void RSAKey::saveKeys() {
    std::filesystem::create_directories(std::filesystem::path(privPath).parent_path());

    FILE* privFile = fopen(privPath.c_str(), "w");
    if (!privFile) handleErrors("Erro ao abrir arquivo de chave privada.");
    if (!PEM_write_PrivateKey(privFile, pkey, nullptr, nullptr, 0, nullptr, nullptr))
        handleErrors("Erro ao salvar chave privada.");
    fclose(privFile);

    FILE* pubFile = fopen(pubPath.c_str(), "w");
    if (!pubFile) handleErrors("Erro ao abrir arquivo de chave pública.");
    if (!PEM_write_PUBKEY(pubFile, pkey))
        handleErrors("Erro ao salvar chave pública.");
    fclose(pubFile);
}

void RSAKey::loadKeys() {
    FILE* privFile = fopen(privPath.c_str(), "r");
    if (!privFile) handleErrors("Erro ao abrir chave privada existente.");

    privKey = PEM_read_PrivateKey(privFile, nullptr, nullptr, nullptr);
    if (!privKey) handleErrors("Erro ao ler chave privada.");
    fclose(privFile);

    FILE* pubFile = fopen(pubPath.c_str(), "r");
    if (!pubFile) handleErrors("Erro ao abrir chave prublica existente.");

    pubKey = PEM_read_PUBKEY(pubFile, nullptr, nullptr, nullptr);
    if (!pubKey) handleErrors("Erro ao ler chave privada.");
    fclose(pubFile);
}

RSAKey::RSAKey(const std::string& dir,
               const std::string& privName,
               const std::string& pubName) {
    privPath = dir + "/" + privName;
    pubPath = dir + "/" + pubName;

    if (std::filesystem::exists(privPath) && std::filesystem::exists(pubPath)) {
        std::cout << "Chaves já existem. Carregando...\n";
        loadKeys();
    } else {
        std::cout << "Chaves não encontradas. Gerando novo par RSA...\n";
        generateRSAKey(2048);
        saveKeys();
    }
}

RSAKey::~RSAKey() {
    if (pkey) EVP_PKEY_free(pkey);
    if (pubKey) EVP_PKEY_free(pubKey);
    if (privKey) EVP_PKEY_free(privKey);
}

EVP_PKEY* RSAKey::getKeyPublic() const {
    return pubKey;
}

EVP_PKEY* RSAKey::getKeyPriv() const {
    return privKey;
}