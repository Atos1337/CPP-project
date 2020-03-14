#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include <assert.h>
#include "openssl/asn1.h"
#include "openssl/bio.h"
#include "openssl/conf.h"
#include "openssl/err.h"
#include "openssl/pem.h"
#include "openssl/x509.h"


// Smart pointers to wrap openssl C types that need explicit free
using BIO_ptr  = std::unique_ptr<BIO, decltype(&BIO_free)>;

class Certificate {
public:
    static void init() {
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
        OPENSSL_no_config();
    }

    static void destroy() {
        FIPS_mode_set(0);
        CONF_modules_unload(1);
        CONF_modules_free();
        ERR_free_strings();
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
    }

    Certificate(std::string filename) {
        BIO_ptr bio_input_ptr(BIO_new(BIO_s_file()), BIO_free);
        if (BIO_read_filename(bio_input_ptr.get(), filename.c_str()) <= 0) {
            throw std::runtime_error("Could not open file");
        }
        cert = PEM_read_bio_X509_AUX(bio_input_ptr.get(), NULL, NULL, NULL);
    }

    ~Certificate() {
        X509_free(cert);
    }

    std::string get_bio() {
        BIO_ptr bio_output_ptr(BIO_new(BIO_s_mem()), BIO_free);
        X509_NAME_print_ex(bio_output_ptr.get(), X509_get_subject_name(cert), 0, 0);
        char buffer[MAX_LEN];
        memset(buffer, 0, MAX_LEN);
        BIO_read(bio_output_ptr.get(), buffer, MAX_LEN - 1);
        return std::string(buffer);
    }

private:
    const size_t MAX_LEN = 4096;
    X509 *cert;
    X509_NAME *subject;
};

int main(int argc, char** argv) {
    Certificate::init();
    if (argc < 2)
    {
        std::cout << "Missing filename" << std::endl;
        return 1;
    }

    Certificate certificate(argv[1]);
    std::cout << certificate.get_bio();
    Certificate::destroy();

    return 0;
}