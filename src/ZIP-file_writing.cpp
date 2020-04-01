#include "ZIP-file_writing.h"
#include "ZIP-file_headers.h"
#include "zlib.h"

#include <cstring>
#include <vector>
#include <cstdint>
#include <fstream>
#include <memory>
#include "openssl/bio.h"
#include "openssl/pem.h"
#include "openssl/x509.h"

namespace ZIP_file_writing{

std::ofstream& operator<<(std::ofstream& out, EOCD& eocd) {
	uint32_t signature = valid_signatures.EOCD;
	out.write(reinterpret_cast<char *>(&signature), sizeof(uint32_t));
	out.write(reinterpret_cast<char *>(&eocd), sizeof(eocd) - sizeof(uint8_t *));
	if (eocd.commentLength) {
		out.write(reinterpret_cast<char *>(eocd.comment.get()), eocd.commentLength);
	}
	return out;
}

std::ofstream& operator<<(std::ofstream& out, CentralDirectoryFileHeader& cdfh) {
	uint32_t signature = valid_signatures.CDFH;
	out.write(reinterpret_cast<char *>(&signature), sizeof(uint32_t));
	out.write(reinterpret_cast<char *>(&cdfh), sizeof(cdfh) - 3 * sizeof(uint8_t *));
	if (cdfh.filenameLength) {
		out.write(reinterpret_cast<char *>(cdfh.filename.get()), cdfh.filenameLength);
	}
	if (cdfh.extraFieldLength) {
		out.write(reinterpret_cast<char *>(cdfh.extraField.get()), cdfh.extraFieldLength);
	}
	if (cdfh.fileCommentLength) {
		out.write(reinterpret_cast<char *>(cdfh.fileComment.get()), cdfh.fileCommentLength);
	}
	return out;
}

std::ofstream& operator<<(std::ofstream& out, DataDescriptor& dd) {
	uint32_t signature = valid_signatures.DD;
	out.write(reinterpret_cast<char *>(&signature), sizeof(uint32_t));
	out.write(reinterpret_cast<char *>(&dd), sizeof(dd));
	return out;
}

std::ofstream& operator<<(std::ofstream& out, LocalFileHeader& lfh) {
	uint32_t signature = valid_signatures.LFH;
	out.write(reinterpret_cast<char *>(&signature), sizeof(uint32_t));
	out.write(reinterpret_cast<char *>(&lfh), sizeof(lfh) - 2 * sizeof(uint8_t *));
	if (lfh.filenameLength) {
		out.write(reinterpret_cast<char *>(lfh.filename.get()), lfh.filenameLength);
	}
	if (lfh.extraFieldLength) {
		out.write(reinterpret_cast<char *>(lfh.extraField.get()), lfh.extraFieldLength);
	}
	return out;
}

std::ofstream& operator<<(std::ofstream& out, File& f) {
	out.write(reinterpret_cast<char *>(f.data.get()), f.compressedSize);
	return out;
}

void deflate_data(File& f) {
	std::unique_ptr<uint8_t[]> data_buf(new uint8_t[f.uncompressedSize]);
	z_stream zs;
	std::memset(&zs, 0, sizeof(zs));
	deflateInit2(
        &zs, 
        Z_BEST_SPEED,
        Z_DEFLATED,
        -MAX_WBITS,
        8,
        Z_DEFAULT_STRATEGY);
	zs.avail_in = f.uncompressedSize;
	zs.next_in = f.data.get();
	zs.avail_out = f.uncompressedSize;
	zs.next_out = data_buf.get();
	deflate(&zs, Z_FINISH);
	f.compressedSize = zs.total_out;
	f.compressionMethod = Z_DEFLATED;
	f.data = std::move(data_buf);
}

std::ofstream& operator<<(std::ofstream& out, const char *certificate) {
	using BIO_ptr = std::unique_ptr<BIO, decltype(&BIO_free)>;
	using X509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;
	uint8_t *buf = nullptr;
	uint32_t size = 0;
	BIO_ptr cert_bio(BIO_new(BIO_s_file()), BIO_free);
	BIO_read_filename(cert_bio.get(), certificate);
	X509_ptr x509(PEM_read_bio_X509_AUX(cert_bio.get(), NULL, 0, NULL), X509_free);
	size = i2d_X509(x509.get(), &buf);
	out.write(reinterpret_cast<char *>(&size), sizeof(uint32_t));
	out.write(reinterpret_cast<char *>(buf), size);
	return out;
}

} //namespace ZIP_file_writing