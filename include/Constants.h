#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

namespace Constants {
    inline const std::string SIGNATUREPATH = "docs/signature/";
    inline const std::string PRIVRSAKEYPATH = "docs/license/RSA/keyPriv.pem";
    inline const std::string PUBLICRSAKEYPATH = "docs/license/RSA/keyPublic.pem";

    inline const std::string ENDPOINT_SIGNATURE = "/signature/";
    inline const std::string ENDPOINT_VERIFY = "/verify/";
    inline const int HTTP_PORT = 8080;
}

#endif
