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

char* readFileBytes(const char *name)  
{  
    std::ifstream fl(name);  
    fl.seekg( 0, std::ios::end );  
    size_t len = fl.tellg();  
    char *ret = new char[len];  
    fl.seekg(0, std::ios::beg);   
    fl.read(ret, len);  
    fl.close();  
    return ret;  
}

X509* getPubKey(std::string filename){
    BIO_ptr bio_input_ptr(BIO_new(BIO_s_file()), BIO_free);
    if (BIO_read_filename(bio_input_ptr.get(), filename.c_str()) <= 0) {
        throw std::runtime_error("Could not open file");
    }
    return PEM_read_bio_X509_AUX(bio_input_ptr.get(), NULL, NULL, NULL);
}

std::string extract_private(std::string filename){
    std::string s(readFileBytes(filename.c_str()));
    return s;
}

std::string extract_public(std::string filename){
    BIO* bio = BIO_new(BIO_s_mem());
    char* buffer;
    PEM_write_bio_PUBKEY(bio, X509_get_pubkey(getPubKey(filename)));
    size_t bioLength = BIO_pending(bio);
    buffer = (char*)malloc(bioLength+1);
    BIO_read(bio, buffer, bioLength);
    buffer[bioLength] = '\0';
    std::string s(buffer);
    return s;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "Missing filename" << std::endl;
        return 1;
    }

    const std::string inputFile1 = argv[1];
    const std::string inputFile2 = argv[2];

    Signer signer;

    std::string privateKey = extract_private(inputFile1);
    std::cout<<privateKey<<"\n";
    std::string publicKey = extract_public(inputFile2);
    std::cout<<publicKey<<"\n";
    std::string plainText = "214124apjhsdfoiquh3487y1fhlsdf\n";
    std::string signature = signer.signMessage(privateKey, plainText);
    //bool authentic = signer.verifySignature(publicKey, "214124apjhsdfoiquh3487y1fhlsdf\n", signature);
    // if ( authentic ) {
    //     std::cout << "Authentic" << std::endl;
    // } 
    // else {
    //     std::cout << "Not Authentic" << std::endl;
    // }
}