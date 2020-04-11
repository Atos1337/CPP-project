#include "ZIP_file_signing.h"
#include <cassert>

int main(int argc, char *argv[]) {
	using ZIP_file_signing::ZIP_file;
	if (argc > 1) {
		ZIP_file z(argv[1]);
		z.load_certificate(argv[2]);
		z.signing();
		bool result = z.check_sign();
		assert(result);
	}
	return 0;
}
