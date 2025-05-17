#ifndef RSA_KEY_H
#define RSA_KEY_H

#include <openssl/evp.h>
#include <string>

class RSAKey {
private:
    EVP_PKEY* pkey;
    EVP_PKEY* pubKey;
    EVP_PKEY* privKey;
    std::string privPath;
    std::string pubPath;

    void handleErrors(const std::string& msg);
    void generateRSAKey(int bits);
    void saveKeys();
    void loadKeys();

public:
    RSAKey(const std::string& dir,
           const std::string& privName = "chave_privada.pem",
           const std::string& pubName  = "chave_publica.pem");
    ~RSAKey();

    EVP_PKEY* getKeyPriv() const;
    EVP_PKEY* getKeyPublic() const;
};

#endif // RSA_KEY_H

