#pragma once
#include <string>
#include <vector>
#include <stdio.h>
#include "ZIP_file_headers.h"
#include "openssl/x509.h"
#include "zipSigner.hpp"

namespace ZIP_file_signing{

class ZIP_file{
public:
	ZIP_file(const char *filename, ZipSigner &signer_);
	//void signing();
	bool check_sign();
	void load_certificate_and_signing(const std::string& certificate);
	std::vector<std::vector<uint8_t>> get_filenames();
private:
	std::string get_result_name(std::string& arch);
	using X509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;
	X509_ptr get_certificate(const CentralDirectoryFileHeader &cdfh);
	std::vector<uint8_t> get_signature(const LocalFileHeader &lfh);
	std::string arch;
	ZipSigner &signer;
};

} //namespace ZIP_file_signing
