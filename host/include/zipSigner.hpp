#pragma once

#include <vector>
#include <unordered_map>
#include "openssl/x509.h"
#include "signer.h"

class ZipSigner {
private:
    std::string privateKey;
    Signer signer;
public:
    ZipSigner(const std::string& privateKey_ = "") : privateKey(privateKey_) {}

    std::vector<uint8_t> sign(const std::vector<uint8_t> &bytes) const ;

    bool verify(X509* certificate,
                const std::vector<uint8_t> &bytes,
                const std::vector<uint8_t> &signature) const ;

    std::unordered_map<std::string, std::string> getCertificateData(X509* certificate) const ;
};
