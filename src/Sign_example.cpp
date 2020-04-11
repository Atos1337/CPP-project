#include "ZIP_file_signing.h"
#include <cassert>
#include <vector>
#include <iostream>

int main(int argc, char *argv[]) {
	using ZIP_file_signing::ZIP_file;
	if (argc > 1) {
		ZIP_file z(argv[1]);
		z.load_certificate(argv[2]);
		z.signing();
		bool result = z.check_sign();
		assert(result);
		std::vector<std::vector<uint8_t>> filenames = z.get_filenames();
		for (size_t i = 0; i < filenames.size(); ++i) {
			std::cout << filenames[i].data() << '\n';
		}
	}
	return 0;
}
