#include <fstream>
#include <iostream>
#include <cstring>
#include <memory>
#include "zlib.h"

#include "ZIP-file_reading.h"
#include "ZIP-file_headers.h"

namespace ZIP_file_reading{

std::ifstream& operator>>(std::ifstream& in, EOCD& eocd) {
	in.seekg(0, in.end);
	size_t filesize = in.tellg();
	for (size_t offset = filesize - sizeof(EOCD) - sizeof(uint32_t) + sizeof(uint8_t *); offset > 0; offset--) {
		uint32_t signature = 0;
		in.seekg(offset, in.beg);
		in.read(reinterpret_cast<char *>(&signature), sizeof(signature));
		if (signature == valid_signatures.EOCD) 
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
	if (signature != valid_signatures.CDFH) {
		std::cerr << "ERROR: CentralDirectoryFileHeader not found!\n";
		//throw
	}
	in.read(reinterpret_cast<char *>(&cdfh), sizeof(CentralDirectoryFileHeader) - 3 * sizeof(uint8_t *));
	if (cdfh.filenameLength) {
		cdfh.filename = std::unique_ptr<uint8_t[]>(new uint8_t[cdfh.filenameLength + 1]);
		in.read(reinterpret_cast<char *>(cdfh.filename.get()), cdfh.filenameLength);
		cdfh.filename[cdfh.filenameLength] = 0;
	}
	
	if (cdfh.extraFieldLength) {
		cdfh.extraField = std::unique_ptr<uint8_t[]>(new uint8_t[cdfh.extraFieldLength]);
		in.read(reinterpret_cast<char *>(cdfh.extraField.get()), cdfh.extraFieldLength);
	}
	if (cdfh.fileCommentLength) {
		cdfh.fileComment = std::unique_ptr<uint8_t[]>(new uint8_t[cdfh.fileCommentLength]);
		in.read(reinterpret_cast<char *>(cdfh.fileComment.get()), cdfh.fileCommentLength);
	}
	return in;
} 

std::ifstream& operator>>(std::ifstream& in, DataDescriptor& dd) {
	uint32_t signature = 0;
	in.read(reinterpret_cast<char *>(&signature), sizeof(signature));
	if (signature != valid_signatures.DD) {
		std::cerr << "ERROR: DataDescriptor not found!\n";
		//throw
	}
	in.read(reinterpret_cast<char *>(&dd), sizeof(DataDescriptor));
	return in;
}

std::ifstream& operator>>(std::ifstream& in, LocalFileHeader& lfh) {
	uint32_t signature = 0;
	in.read(reinterpret_cast<char *>(&signature), sizeof(signature));
	if (signature != valid_signatures.LFH) {
		std::cerr << "ERROR: LocalFileHeader not found!\n";
		//throw
	}
	in.read(reinterpret_cast<char *>(&lfh), sizeof(LocalFileHeader) - 2 * sizeof(uint8_t *));
	if (lfh.filenameLength) {
		lfh.filename = std::unique_ptr<uint8_t[]>(new uint8_t[lfh.filenameLength + 1]);
		in.read(reinterpret_cast<char *>(lfh.filename.get()), lfh.filenameLength);
		lfh.filename[lfh.filenameLength] = 0;
	}
	if (lfh.extraFieldLength) {
		lfh.extraField = std::unique_ptr<uint8_t[]>(new uint8_t[lfh.extraFieldLength]);
		in.read(reinterpret_cast<char *>(lfh.extraField.get()), lfh.extraFieldLength);
	}
	return in;	
}

void inflate_data(File& f) {
	if (Z_DEFLATED == f.compressionMethod) {
		std::unique_ptr<uint8_t[]> result (new uint8_t[f.uncompressedSize]);
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
	std::unique_ptr<uint8_t[]> read_buf(new uint8_t[f.compressedSize]);
	in.read(reinterpret_cast<char *>(read_buf.get()), f.compressedSize);
	f.data = std::move(read_buf);
	return in;
}

std::ifstream& operator>>(std::ifstream& in, std::ofstream& out) {
	uint32_t size = 0;
	in.read(reinterpret_cast<char *>(&size), sizeof(uint32_t));
	std::unique_ptr<uint8_t[]> sertificate(new uint8_t[size]);
	out.write(reinterpret_cast<char *>(sertificate.get()), size);
	return in;
}

} //namespace ZIP_file_reading
