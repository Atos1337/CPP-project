#pragma once
#include <vector>
#include "ZIP_file_headers.h"
#include "openssl/x509.h"

std::vector<uint8_t> test_sign(File &f);

using X509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;

bool ch_sign(File &f, const std::vector<uint8_t> &sign, X509* x509);
