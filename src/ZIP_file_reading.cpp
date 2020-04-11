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
	for (size_t offset = filesize - sizeof(EOCD) - sizeof(uint32_t) + sizeof(uint8_t *); offset > 0; offset--) {
		uint32_t signature = 0;
		in.seekg(offset, in.beg);
		in.read(reinterpret_cast<char *>(&signature), sizeof(signature));
		if (signature == static_cast<uint32_t>(valid_signatures::EOCD)) 
			break;
	}
	in.read(reinterpret_cast<char *>(&eocd), sizeof(EOCD) - sizeof(uint8_t *));
	if (eocd.commentLength) {
		eocd.comment = std::unique_ptr<uint8_t[]>(new uint8_t[eocd.commentLength]);
		in.read(reinterpret_cast<char *>(eocd.comment.get()), eocd.commentLength);
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
	in.read(reinterpret_cast<char *>(&cdfh), sizeof(cdfh) - sizeof(uint16_t) - 3 * sizeof(uint8_t *));
	if (cdfh.filenameLength) {
		cdfh.filename = std::make_unique<uint8_t[]>(cdfh.filenameLength + 1);
		in.read(reinterpret_cast<char *>(cdfh.filename.get()), cdfh.filenameLength);
		cdfh.filename[cdfh.filenameLength] = 0;
	}
	if (cdfh.extraFieldLength) {
		cdfh.totalExtraFieldRecord = 0;
		uint32_t extrafield_offset = in.tellg();
		std::unique_ptr<extraFieldRecord[]> tmp = std::make_unique<extraFieldRecord[]>(cdfh.extraFieldLength/(2 * sizeof(uint16_t)));
		for (size_t offset = extrafield_offset; offset - extrafield_offset < cdfh.extraFieldLength; ++cdfh.totalExtraFieldRecord) {
			extraFieldRecord efr;
			in.read(reinterpret_cast<char *>(&efr.signature), sizeof(uint16_t));
			in.read(reinterpret_cast<char *>(&efr.size), sizeof(uint16_t));
			efr.data = std::make_unique<uint8_t[]>(efr.size);
			in.read(reinterpret_cast<char *>(efr.data.get()), efr.size);
			offset += 2 * sizeof(uint16_t) + efr.size;
			tmp[cdfh.totalExtraFieldRecord] = std::move(efr);
		}
		cdfh.extraField = std::make_unique<extraFieldRecord[]>(cdfh.totalExtraFieldRecord);
		for (int i = 0; i < cdfh.totalExtraFieldRecord; ++i) {
			cdfh.extraField[i] = std::move(tmp[i]);
		}
	}
	if (cdfh.fileCommentLength) {
		cdfh.fileComment = std::make_unique<uint8_t[]>(cdfh.fileCommentLength);
		in.read(reinterpret_cast<char *>(cdfh.fileComment.get()), cdfh.fileCommentLength);
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
	in.read(reinterpret_cast<char *>(&lfh), sizeof(lfh) - 2 * sizeof(uint8_t *) - sizeof(uint16_t));
	if (lfh.filenameLength) {
		lfh.filename = std::make_unique<uint8_t[]>(lfh.filenameLength + 1);
		in.read(reinterpret_cast<char *>(lfh.filename.get()), lfh.filenameLength);
		lfh.filename[lfh.filenameLength] = 0;
	}
	if (lfh.extraFieldLength) {
		lfh.totalExtraFieldRecord = 0;
		uint32_t extrafield_offset = in.tellg();
		std::unique_ptr<extraFieldRecord[]> tmp = std::make_unique<extraFieldRecord[]>(lfh.extraFieldLength/(2 * sizeof(uint16_t)));
		for (size_t offset = extrafield_offset; offset - extrafield_offset < lfh.extraFieldLength; ++lfh.totalExtraFieldRecord) {
			extraFieldRecord efr;
			in.read(reinterpret_cast<char *>(&efr.signature), sizeof(uint16_t));
			in.read(reinterpret_cast<char *>(&efr.size), sizeof(uint16_t));
			efr.data = std::make_unique<uint8_t[]>(efr.size);
			in.read(reinterpret_cast<char *>(efr.data.get()), efr.size);
			offset += 2 * sizeof(uint16_t) + efr.size;
			tmp[lfh.totalExtraFieldRecord] = std::move(efr);
		}
		lfh.extraField = std::make_unique<extraFieldRecord[]>(lfh.totalExtraFieldRecord);
		for (int i = 0; i < lfh.totalExtraFieldRecord; ++i) {
			lfh.extraField[i] = std::move(tmp[i]);
		}
	}
	return in;	
}

void inflate_data(File& f) {
	if (Z_DEFLATED == f.compressionMethod) {
		std::unique_ptr<uint8_t[]> result = std::make_unique<uint8_t[]>(f.uncompressedSize);
		z_stream zs;
		std::memset(&zs, 0, sizeof(zs));
		inflateInit2(&zs, -MAX_WBITS);
		zs.avail_in = f.compressedSize;
		zs.next_in = f.data.get();
		zs.avail_out = f.uncompressedSize;
		zs.next_out = result.get();
		inflate(&zs, Z_FINISH);
		inflateEnd(&zs);
		f.data = std::move(result);
	}	
}

std::ifstream& operator>>(std::ifstream& in, File& f) {
	f.data = std::make_unique<uint8_t[]>(f.compressedSize);
	in.read(reinterpret_cast<char *>(f.data.get()), f.compressedSize);
	return in;
}

std::ifstream& operator>>(std::ifstream& in, std::ofstream& out) {
	using BIO_ptr = std::unique_ptr<BIO, decltype(&BIO_free)>;
	using X509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;
	uint16_t size;
	in.read(reinterpret_cast<char *>(&size), sizeof(uint16_t));
	uint8_t *buf = new uint8_t[size];
	in.read(reinterpret_cast<char *>(buf), size);
	X509_ptr x509(d2i_X509(NULL, const_cast<const uint8_t **>(&buf), size), X509_free);
	BIO_ptr cert_bio(BIO_new(BIO_s_mem()), BIO_free);
	PEM_write_bio_X509(cert_bio.get(), x509.get());
	BUF_MEM *mem = NULL;
    BIO_get_mem_ptr(cert_bio.get(), &mem);
    out.write(reinterpret_cast<char *>(mem->data), mem->length);
	return in;
}

} //namespace ZIP_file_reading
