#include <fstream>
#include <iostream>
#include <cstring>
#include <ios>
#include <memory>
#include <cassert>
#include "zlib.h"
#include "openssl/bio.h"
#include "openssl/pem.h"
#include "openssl/x509.h"

#include "ZIP_file_reading.h"
#include "ZIP_file_headers.h"
#include "ZIP_file_signing.h"

namespace ZIP_file_reading{

std::ifstream& operator>>(std::ifstream& in, EOCD& eocd) {
	in.seekg(0, in.end);
	size_t filesize = in.tellg();
	for (size_t offset = filesize - sizeof(eocd) - sizeof(uint32_t) + sizeof(eocd.comment); offset > 0; offset--) {
		uint32_t signature = 0;
		in.seekg(offset, in.beg);
		in.read(reinterpret_cast<char *>(&signature), sizeof(signature));
		if (signature == static_cast<uint32_t>(valid_signatures::EOCD)) 
			break;
	}
	in.read(reinterpret_cast<char *>(&eocd), sizeof(eocd) - sizeof(eocd.comment));
	if (eocd.commentLength) {
		eocd.comment.resize(eocd.commentLength);
		in.read(reinterpret_cast<char *>(eocd.comment.data()), eocd.commentLength);
	}
	return in;
}

std::ifstream& operator>>(std::ifstream& in, CentralDirectoryFileHeader& cdfh) {
	uint32_t signature = 0;
	in.read(reinterpret_cast<char *>(&signature), sizeof(signature));
	if (signature != static_cast<uint32_t>(valid_signatures::CDFH)) {
		std::cerr << "ERROR: CentralDirectoryFileHeader not found!\n";
		//throw
	}
	in.read(reinterpret_cast<char *>(&cdfh), sizeof(cdfh) - sizeof(cdfh.extraField) - sizeof(cdfh.fileComment) - sizeof(cdfh.filename));
	if (cdfh.filenameLength) {
		cdfh.filename.resize(cdfh.filenameLength + 1);
		in.read(reinterpret_cast<char *>(cdfh.filename.data()), cdfh.filenameLength);
		cdfh.filename[cdfh.filenameLength] = 0;
	}
	if (cdfh.extraFieldLength) {
		uint32_t extrafield_offset = in.tellg();
		for (size_t offset = extrafield_offset; offset - extrafield_offset < cdfh.extraFieldLength;) {
			extraFieldRecord efr;
			in.read(reinterpret_cast<char *>(&efr.signature), sizeof(uint16_t));
			in.read(reinterpret_cast<char *>(&efr.size), sizeof(uint16_t));
			efr.data.resize(efr.size);
			in.read(reinterpret_cast<char *>(efr.data.data()), efr.size);
			offset += 2 * sizeof(uint16_t) + efr.size;
			cdfh.extraField.push_back(std::move(efr));
		}
	}
	if (cdfh.fileCommentLength) {
		cdfh.fileComment.resize(cdfh.fileCommentLength);
		in.read(reinterpret_cast<char *>(cdfh.fileComment.data()), cdfh.fileCommentLength);
	}
	return in;
} 

std::ifstream& operator>>(std::ifstream& in, DataDescriptor& dd) {
	uint32_t signature = 0;
	in.read(reinterpret_cast<char *>(&signature), sizeof(signature));
	if (signature != static_cast<uint32_t>(valid_signatures::DD)) {
		std::cerr << "ERROR: DataDescriptor not found!\n";
		//throw
	}
	in.read(reinterpret_cast<char *>(&dd), sizeof(dd));
	return in;
}

std::ifstream& operator>>(std::ifstream& in, LocalFileHeader& lfh) {
	uint32_t signature = 0;
	in.read(reinterpret_cast<char *>(&signature), sizeof(signature));
	if (signature != static_cast<uint32_t>(valid_signatures::LFH)) {
		std::cerr << "ERROR: LocalFileHeader not found!\n";
		//throw
	}
	in.read(reinterpret_cast<char *>(&lfh), sizeof(lfh) - sizeof(lfh.filename) - sizeof(lfh.extraField));
	if (lfh.filenameLength) {
		lfh.filename.resize(lfh.filenameLength + 1);
		in.read(reinterpret_cast<char *>(lfh.filename.data()), lfh.filenameLength);
		lfh.filename[lfh.filenameLength] = 0;
	}
	if (lfh.extraFieldLength) {
		uint32_t extrafield_offset = in.tellg();
		for (size_t offset = extrafield_offset; offset - extrafield_offset < lfh.extraFieldLength;) {
			extraFieldRecord efr;
			in.read(reinterpret_cast<char *>(&efr.signature), sizeof(uint16_t));
			in.read(reinterpret_cast<char *>(&efr.size), sizeof(uint16_t));
			efr.data.resize(efr.size);
			in.read(reinterpret_cast<char *>(efr.data.data()), efr.size);
			offset += 2 * sizeof(uint16_t) + efr.size;
			lfh.extraField.push_back(std::move(efr));
		}
	}
	return in;	
}

void inflate_data(File& f) {
	if (Z_DEFLATED == f.compressionMethod) {
		std::vector<uint8_t> result(f.uncompressedSize);
		z_stream zs;
		std::memset(&zs, 0, sizeof(zs));
		inflateInit2(&zs, -MAX_WBITS);
		zs.avail_in = f.compressedSize;
		zs.next_in = f.data.data();
		zs.avail_out = f.uncompressedSize;
		zs.next_out = result.data();
		inflate(&zs, Z_FINISH);
		inflateEnd(&zs);
		f.data = std::move(result);
	}	
}

std::ifstream& operator>>(std::ifstream& in, File& f) {
	f.data.resize(f.compressedSize);
	in.read(reinterpret_cast<char *>(f.data.data()), f.compressedSize);
	return in;
}

auto deserialize(std::vector<uint8_t>& certificate) -> X509_ptr {
	uint8_t *buf = certificate.data();
	X509_ptr x509(d2i_X509(NULL, const_cast<const uint8_t **>(&buf), certificate.size()), X509_free);
	return std::move(x509);
}

} //namespace ZIP_file_reading
