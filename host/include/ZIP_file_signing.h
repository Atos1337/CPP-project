#pragma once
#include <string>
#include <vector>
#include "ZIP_file_headers.h"
#include "openssl/x509.h"

namespace ZIP_file_signing{

class ZIP_file{
public:
	ZIP_file(const char *filename);
	void signing();
	bool check_sign();
	void load_certificate(const char *certificate);
	std::vector<std::vector<uint8_t>> get_filenames();
private:
	using X509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;
	X509_ptr get_certificate(const CentralDirectoryFileHeader &cdfh);
	std::vector<uint8_t> get_signature(const LocalFileHeader &lfh);
	std::string arch;
};

} //namespace ZIP_file_signing
