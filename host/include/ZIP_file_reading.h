#pragma once

#include <fstream>
#include <vector>

#include "ZIP_file_headers.h"
#include "zlib.h"
#include "openssl/x509.h"

namespace ZIP_file_reading {

std::ifstream& operator>>(std::ifstream& in, EOCD& eocd);

std::ifstream& operator>>(std::ifstream& in, EOCD64Locator& locator);

std::ifstream& operator>>(std::ifstream& in, EOCD64& eocd64);

std::ifstream& operator>>(std::ifstream& in, CentralDirectoryFileHeader& cdfh);

std::ifstream& operator>>(std::ifstream& in, DataDescriptor& dd);

std::ifstream& operator>>(std::ifstream& in, LocalFileHeader& lfh);

void inflate_data(File& f);

std::ifstream& operator>>(std::ifstream& in, File& f);

using X509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;

auto deserialize(std::vector<uint8_t>& certificate) -> X509_ptr;

std::string get_certificate_to_check(X509_ptr x509);

} //namespace ZIP_file_reading