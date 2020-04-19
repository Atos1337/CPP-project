#include "zipSigner.hpp"

std::vector<uint8_t> ZipSigner::sign(const std::vector<uint8_t> &bytes) {
    std::string msg = signer.signMessage(privateKey, std::string(bytes.cbegin(), bytes.cend()));
    return std::vector<uint8_t>(msg.begin(), msg.end());
}

bool ZipSigner::verify(X509* certificate,
                       const std::vector<uint8_t> &bytes,
                       const std::vector<uint8_t> &signature) {
    assert(certificate);
    return verifySignature(certificate,
                           std::string(bytes.cbegin(), bytes.cend()),
                           std::string(signature.cbegin(), signature.cend()));
}