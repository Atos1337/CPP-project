#pragma once

class Signer{
public:
	Signer();
	~Signer();
	std::string signMessage(std::string privateKey, std::string plainText);
	bool verifySignature(std::string publicKey, std::string plainText, std::string signatureBase64);
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
	
};

char* readFileBytes(const char *name);
X509* getPubKey(std::string filename);
std::string extract_private(std::string filename);
std::string extract_public(std::string filename);