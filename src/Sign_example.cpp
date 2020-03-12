#include "ZIP-file_signing.h"

int main(int argc, char *argv[]) {
	using ZIP_file_signing::ZIP_file;
	if (argc > 2) {
		ZIP_file z(argv[1]);
		z.signing(argv[2]);
		z.get_and_write_sertificate(argv[3]);
	}
	return 0;
}
