#pragma once

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

class Signer{
public:
	Signer();
	~Signer();
	std::string signMessage(std::string privateKey, std::string plainText);
	bool verifySignature(X509* certificate, std::string plainText, std::string signatureBase64);
private:
	RSA* createPrivateRSA(std::string key);
	RSA* createPublicRSA(std::string key);
	bool RSASign( RSA* rsa,
              const unsigned char* Msg,
              size_t MsgLen,
              unsigned char** EncMsg,
              size_t* MsgLenEnc);
	bool RSAVerifySignature( RSA* rsa,
                         unsigned char* MsgHash,
                         size_t MsgHashLen,
                         const char* Msg,
                         size_t MsgLen,
                         bool* Authentic);
	void Base64Encode( const unsigned char* buffer,
                   size_t length,
                   char** base64Text);
	size_t calcDecodeLength(const char* b64input);
	void Base64Decode(const char* b64message, unsigned char** buffer, size_t* length);
    std::string extractPublicKey(X509 *certificate);
};