#include <vector>
#include <iostream>
#include "ZIP_file_headers.h"
#include "Test_function_for_sign.h"
#include "openssl/x509.h"

std::vector<uint8_t> test_sign(File &f) {
	static_cast<void>(f);
	std::vector<uint8_t> res = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0};
	return std::move(res);
}

bool ch_sign(File &f, const std::vector<uint8_t> &sign, X509* x509) {
	static_cast<void>(x509);
	static_cast<void>(f);
	std::vector<uint8_t> res = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0};
	return sign == res;
}
