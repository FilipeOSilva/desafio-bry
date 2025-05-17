#ifndef RSA_KEY_H
#define RSA_KEY_H

#include <openssl/evp.h>
#include <string>
#include "Constants.h"

class RSAKey {
private:
    EVP_PKEY* pkey = nullptr;
    EVP_PKEY* pubKey = nullptr;
    EVP_PKEY* privKey = nullptr;
    const std::string privPath = Constants::PRIVRSAKEYPATH;
    const std::string pubPath = Constants::PUBLICRSAKEYPATH;

    void handleErrors(const std::string& msg);
    void generateRSAKey(int bits);
    void saveKeys();
    void loadKeys();

public:
    RSAKey();

    EVP_PKEY* getKeyPriv() const;
    EVP_PKEY* getKeyPublic() const;
};

#endif // RSA_KEY_H

