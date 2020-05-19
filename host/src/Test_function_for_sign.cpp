#include <vector>
#include <iostream>
#include "ZIP_file_headers.h"
#include "Test_function_for_sign.h"
#include "openssl/x509.h"

std::vector<uint8_t> test_sign(std::vector<uint8_t> &f) {
	static_cast<void>(f);
	std::vector<uint8_t> res = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0};
	return std::move(res);
}

bool ch_sign(X509* x509, const std::vector<uint8_t> &f, const std::vector<uint8_t> &sign) {
	static_cast<void>(x509);
	static_cast<void>(f);
	std::vector<uint8_t> res = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', 0};
	return sign == res;
}
