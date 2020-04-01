#include "ZIP-file_signing.h"
#include "ZIP-file_reading.h"
#include "ZIP-file_writing.h"

#include <iostream>

namespace ZIP_file_signing {
	using namespace ZIP_file_reading;
	using namespace ZIP_file_writing;

	ZIP_file::ZIP_file(const char *filename) : arch(filename) {}

	void ZIP_file::signing(const char *sertificate) {
		std::ifstream in(arch.c_str(), std::ios::binary);
		std::unique_ptr<EOCD> eocd(new EOCD);
		in >> *eocd;
		std::unique_ptr<CentralDirectoryFileHeader[]> cdfh(new CentralDirectoryFileHeader[eocd->totalCentralDirectoryRecord]);
		in.seekg(eocd->centralDirectoryOffset, in.beg);
		for (int i = 0; i < eocd->totalCentralDirectoryRecord; i++) {
			in >> cdfh[i];
		}
		std::unique_ptr<LocalFileHeader[]> lfh(new LocalFileHeader[eocd->totalCentralDirectoryRecord]);
		std::unique_ptr<File[]> f(new File[eocd->totalCentralDirectoryRecord]);
		for (int i = 0; i < eocd->totalCentralDirectoryRecord; i++) {
			in.seekg(cdfh[i].localFileHeaderOffset, in.beg);
			in >> lfh[i];
			f[i] = {std::unique_ptr<uint8_t[]>(nullptr), lfh[i].compressionMethod, lfh[i].compressedSize, lfh[i].uncompressedSize};
			in >> f[i];
		}
		in.close();
		std::ofstream out(arch.c_str(), std::ios::binary);
		if (!out) {
			std::cerr << "ERROR: Unable to open file\n";
		}
		for (int i = 0; i < eocd->totalCentralDirectoryRecord; i++) {
			out << lfh[i] << f[i];
		}
		out << sertificate;
		eocd->centralDirectoryOffset = out.tellp();
		for (int i = 0; i < eocd->totalCentralDirectoryRecord; i++) {
			out << cdfh[i];
		}
		out << *eocd;
	}

	void ZIP_file::get_and_write_sertificate(const char *filename) {
		std::ifstream in(arch.c_str(), std::ios::binary);
		std::unique_ptr<EOCD> eocd(new EOCD);
		in >> *eocd;
		std::unique_ptr<CentralDirectoryFileHeader[]> cdfh(new CentralDirectoryFileHeader[eocd->totalCentralDirectoryRecord]);
		in.seekg(eocd->centralDirectoryOffset, in.beg);
		for (int i = 0; i < eocd->totalCentralDirectoryRecord; i++) {
			in >> cdfh[i];
		}
		std::unique_ptr<LocalFileHeader[]> lfh(new LocalFileHeader[eocd->totalCentralDirectoryRecord]);
		std::unique_ptr<File[]> f(new File[eocd->totalCentralDirectoryRecord]);
		for (int i = 0; i < eocd->totalCentralDirectoryRecord; i++) {
			in.seekg(cdfh[i].localFileHeaderOffset, in.beg);
			in >> lfh[i];
			f[i] = {std::unique_ptr<uint8_t[]>(nullptr), lfh[i].compressionMethod, lfh[i].compressedSize, lfh[i].uncompressedSize};
			in >> f[i];
		}
		std::ofstream file(filename, std::ios::binary);
		in >> file;
	}
} //namespace ZIP_file_signing
