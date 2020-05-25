#include "zipSigner.hpp"

std::vector<uint8_t> ZipSigner::sign(const std::vector<uint8_t> &bytes) {
    std::string msg = signer.signMessage(privateKey, std::string(bytes.cbegin(), bytes.cend()));
    return std::vector<uint8_t>(msg.begin(), msg.end());
}

bool ZipSigner::verify(X509* certificate,
                       const std::vector<uint8_t> &bytes,
                       const std::vector<uint8_t> &signature) {
    assert(certificate);
    return signer.verifySignature(certificate,
                           std::string(bytes.cbegin(), bytes.cend()),
                           std::string(signature.cbegin(), signature.cend()));
}

std::unordered_map<std::string, std::string> ZipSigner::getCertificateData(X509* certificate) {
    std::string s = signer.getCertificateData(certificate);
    std::string delimiter = ", ";
    std::vector<std::string> token(7);
    for(int i = 0; i < 5; i++){
        token[i] = s.substr(0, s.find(delimiter));
        s = s.substr(s.find(delimiter) + 2, s.length());
    }
    delimiter = "/";
    token[5] = s.substr(0, s.find(delimiter));
    s = s.substr(s.find(delimiter) + 1,s.length());
    delimiter = " ";
    token[6] = s.substr(0, s.find(delimiter));
    s = s.substr(s.find(delimiter) + 1,s.length());
    std::vector<std::string> unit(7);
    delimiter = "=";
    std::unordered_map<std::string, std::string> res;
    for(int i = 0; i < 7; i++){
        unit[i] = token[i].substr(token[i].find(delimiter)+1,token[i].length());
        token[i] = token[i].substr(0, token[i].find(delimiter));
        res[token[i]] = unit[i];
    }
    return res;
}