#include "signer.h"

using BIO_ptr  = std::unique_ptr<BIO, decltype(&BIO_free)>;

RSA* Signer::createPrivateRSA(std::string key) const {
    RSA *rsa = NULL;
    const char* c_string = key.c_str();
    BIO * keybio = BIO_new_mem_buf((void*)c_string, -1);
    if (keybio==NULL) {
        return 0;
    }
    rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa,NULL, NULL);
    return rsa;
}

RSA* Signer::createPublicRSA(std::string key) const {
    RSA *rsa = NULL;
    BIO *keybio;
    const char* c_string = key.c_str();
    keybio = BIO_new_mem_buf((void*)c_string, -1);
    if (keybio==NULL) {
        return 0;
    }
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa,NULL, NULL);
    return rsa;
}

bool Signer::RSASign( RSA* rsa,
              const unsigned char* Msg,
              size_t MsgLen,
              unsigned char** EncMsg,
              size_t* MsgLenEnc) const {
    EVP_MD_CTX* m_RSASignCtx = EVP_MD_CTX_create();
    EVP_PKEY* priKey  = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(priKey, rsa);
    if (EVP_DigestSignInit(m_RSASignCtx,NULL, EVP_sha256(), NULL,priKey)<=0) {
        return false;
    }
    if (EVP_DigestSignUpdate(m_RSASignCtx, Msg, MsgLen) <= 0) {
        return false;
    }
    if (EVP_DigestSignFinal(m_RSASignCtx, NULL, MsgLenEnc) <=0) {
        return false;
    }
    *EncMsg = (unsigned char*)malloc(*MsgLenEnc);
    if (EVP_DigestSignFinal(m_RSASignCtx, *EncMsg, MsgLenEnc) <= 0) {
        return false;
    }
    EVP_MD_CTX_cleanup(m_RSASignCtx);
    return true;
}

bool Signer::RSAVerifySignature( RSA* rsa,
                         unsigned char* MsgHash,
                         size_t MsgHashLen,
                         const char* Msg,
                         size_t MsgLen,
                         bool* Authentic) const {
    *Authentic = false;
    EVP_PKEY* pubKey  = EVP_PKEY_new();
    EVP_PKEY_assign_RSA(pubKey, rsa);
    EVP_MD_CTX* m_RSAVerifyCtx = EVP_MD_CTX_create();

    if (EVP_DigestVerifyInit(m_RSAVerifyCtx,NULL, EVP_sha256(),NULL,pubKey)<=0) {
        return false;
    }
    if (EVP_DigestVerifyUpdate(m_RSAVerifyCtx, Msg, MsgLen) <= 0) {
        return false;
    }
    int AuthStatus = EVP_DigestVerifyFinal(m_RSAVerifyCtx, MsgHash, MsgHashLen);
    if (AuthStatus==1) {
        *Authentic = true;
        EVP_MD_CTX_cleanup(m_RSAVerifyCtx);
        return true;
    } 
    else if(AuthStatus==0){
        *Authentic = false;
        EVP_MD_CTX_cleanup(m_RSAVerifyCtx);
        return true;    
    } 
    else {
        *Authentic = false;
        EVP_MD_CTX_cleanup(m_RSAVerifyCtx);
        return false;
    }
}

void Signer::Base64Encode(const unsigned char* buffer,
                   size_t length,
                   char** base64Text) const {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);
    BIO_set_close(bio, BIO_NOCLOSE);
    BIO_free_all(bio);

    *base64Text=(*bufferPtr).data;
}

size_t Signer::calcDecodeLength(const char* b64input) const {
    size_t len = strlen(b64input), padding = 0;

    if (b64input[len-1] == '=' && b64input[len-2] == '=') 
        padding = 2;
    else if (b64input[len-1] == '=') 
        padding = 1;
    return (len*3)/4 - padding;
}

void Signer::Base64Decode(const char* b64message, unsigned char** buffer, size_t* length) const {
    BIO *bio, *b64;

    int decodeLen = calcDecodeLength(b64message);
    *buffer = (unsigned char*)malloc(decodeLen + 1);
    (*buffer)[decodeLen] = '\0';

    bio = BIO_new_mem_buf(b64message, -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    *length = BIO_read(bio, *buffer, strlen(b64message));
    BIO_free_all(bio);
}

std::string Signer::signMessage(std::string privateKey, std::string plainText) const {
    RSA* privateRSA = createPrivateRSA(privateKey); 
    unsigned char* encMessage;
    char* base64Text;
    size_t encMessageLength;
    RSASign(privateRSA, (unsigned char*) plainText.c_str(), plainText.length(), &encMessage, &encMessageLength);
    Base64Encode(encMessage, encMessageLength, &base64Text);
    free(encMessage);
    std::string s(base64Text);
    return s;
}

std::string Signer::extractPublicKey(X509* certificate) const {
    assert(certificate);
    BIO* bio = BIO_new(BIO_s_mem());
    char* buffer;
    PEM_write_bio_PUBKEY(bio, X509_get_pubkey(certificate));
    size_t bioLength = BIO_pending(bio);
    buffer = (char*)malloc(bioLength + 1);
    BIO_read(bio, buffer, bioLength);
    buffer[bioLength] = '\0';
    std::string s(buffer);
    return s;
}

bool Signer::verifySignature(X509* certificate, std::string plainText, std::string signatureBase64) const {
    assert(certificate);
    std::string publicKey = extractPublicKey(certificate);
    RSA* publicRSA = createPublicRSA(publicKey);
    unsigned char* encMessage;
    size_t encMessageLength;
    bool authentic;
    Base64Decode(signatureBase64.c_str(), &encMessage, &encMessageLength);
    bool result = RSAVerifySignature(publicRSA, encMessage, encMessageLength, plainText.c_str(), plainText.length(), &authentic);
    return result & authentic;
}

std::string Signer::getCertificateData(X509* certificate) const {
    assert(certificate);
    BIO_ptr bio_output_ptr(BIO_new(BIO_s_mem()), BIO_free);
    X509_NAME_print_ex(bio_output_ptr.get(), X509_get_subject_name(certificate), 0, 0);
    char buffer[MAX_BIO_LEN];
    memset(buffer, 0, MAX_BIO_LEN);
    BIO_read(bio_output_ptr.get(), buffer, MAX_BIO_LEN - 1);
    return std::string(buffer);
}