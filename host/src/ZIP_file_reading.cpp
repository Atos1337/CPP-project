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

std::ifstream& operator>>(std::ifstream& in, EOCD64Locator& locator) {
	in.seekg(0, in.end);
	size_t filesize = in.tellg();
	size_t offset;
	for (offset = filesize - sizeof(EOCD) - sizeof(uint32_t) + sizeof(std::vector<uint8_t>); offset > 0; offset--) {
		uint32_t signature = 0;
		in.seekg(offset, in.beg);
		in.read(reinterpret_cast<char *>(&signature), sizeof(signature));
		if (signature == static_cast<uint32_t>(valid_signatures::EOCD)) 
			break;
	}
	for (offset = offset - sizeof(locator); offset > 0; offset--) {
		uint32_t signature = 0;
		in.seekg(offset, in.beg);
		in.read(reinterpret_cast<char *>(&signature), sizeof(signature));
		if (signature == static_cast<uint32_t>(valid_signatures::Locator)) 
			break;
	}
	in.read(reinterpret_cast<char *>(&locator), sizeof(locator));
	return in;
}

std::ifstream& operator>>(std::ifstream& in, EOCD64& eocd64) {
	uint32_t signature;
	in.read(reinterpret_cast<char *>(&signature), sizeof(signature));
	if (signature != static_cast<uint32_t>(valid_signatures::EOCD64)) {
		std::cerr << "ERROR: EOCD64 not found!\n";
		//throw
	}
	in.read(reinterpret_cast<char*>(&eocd64), sizeof(eocd64) - sizeof(eocd64.data_sector));
	uint64_t size = eocd64.eocd64Size + 12 - 28;
	if (size) {
		in.read(reinterpret_cast<char *>(eocd64.data_sector.data()), size);
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

std::string get_certificate_to_check(X509_ptr x509) {
	using BIO_ptr = std::unique_ptr<BIO, decltype(&BIO_free)>;
	BIO_ptr cert_bio(BIO_new(BIO_s_mem()), BIO_free);
	PEM_write_bio_X509(cert_bio.get(), x509.get());
	BUF_MEM *mem = nullptr;
    BIO_get_mem_ptr(cert_bio.get(), &mem);
    return {reinterpret_cast<char *>(mem->data), mem->length - 1};
}

} //namespace ZIP_file_reading
