#include "ZIP-file_writing.h"
#include "ZIP-file_headers.h"
#include "zlib.h"

#include <cstring>
#include <vector>
#include <cstdint>
#include <fstream>
#include <memory>

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

std::ofstream& operator<<(std::ofstream& out, std::ifstream& in) {
	in.seekg(0, in.end);
	uint32_t size_sertificate = in.tellg();
	in.seekg(0, in.beg);
	std::unique_ptr<uint8_t[]> read_buf(new uint8_t[size_sertificate]);
	in.read(reinterpret_cast<char *>(read_buf.get()), size_sertificate);
	out.write(reinterpret_cast<char *>(&size_sertificate), sizeof(uint32_t));
	out.write(reinterpret_cast<char *>(read_buf.get()), size_sertificate);
	return out;
}

} //namespace ZIP_file_writing