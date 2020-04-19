#pragma once

#include <vector>
#include "openssl/x509.h"
#include "signer.h"

class ZipSigner {
private:
    std::string privateKey;
    Signer& signer;
public:
    ZipSigner(const std::string& privateKey_, Signer& signer_) : privateKey(privateKey_), signer(signer_) {}

    std::vector<uint8_t> sign(const std::vector<uint8_t> &bytes);

    bool verify(X509* certificate,
                const std::vector<uint8_t> &bytes,
                const std::vector<uint8_t> &signature);
};
