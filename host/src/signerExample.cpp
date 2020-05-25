#include <iostream>
#include <fstream>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/engine.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <assert.h>
#include "openssl/asn1.h"
#include "openssl/conf.h"
#include "openssl/err.h"
#include "openssl/x509.h"
#include <memory>
#include <sstream>
#include "signer.h"

using BIO_ptr  = std::unique_ptr<BIO, decltype(&BIO_free)>;


X509* getPubKey(std::string filename){
    BIO_ptr bio_input_ptr(BIO_new(BIO_s_file()), BIO_free);
    if (BIO_read_filename(bio_input_ptr.get(), filename.c_str()) <= 0) {
        throw std::runtime_error("Could not open file");
    }
    return PEM_read_bio_X509_AUX(bio_input_ptr.get(), NULL, NULL, NULL);
}


int main(int argc, char** argv) {
    Signer signer;
    const std::string inputFile = argv[1];
    auto res = signer.getCertificateData(getPubKey(inputFile));
    std::cout << res << std::endl;
}