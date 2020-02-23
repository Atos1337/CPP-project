#include <fstream>
#include <iostream>
#include <cstring>
#include "zlib.h"

#include "ZIP-file_reading.h"
#include "ZIP-file_headers.h"

EOCD* read_eocd(std::ifstream& in) {
	EOCD *eocd = new EOCD;
	in.seekg(0, in.end);
	size_t filesize = in.tellg();
	for (size_t offset = filesize - sizeof(EOCD) - sizeof(uint32_t) + sizeof(uint8_t *); offset > 0; offset--) {
		uint32_t signature = 0;
		in.seekg(offset, in.beg);
		in.read((char *)&signature, sizeof(signature));
		if (signature == valid_signatures.EOCD) 
			break;
	}
	in.read((char *)eocd, sizeof(EOCD) - sizeof(uint8_t *));
	if (eocd->commentLength) {
		eocd->comment = new uint8_t[eocd->commentLength + 1];
		in.read((char *)eocd->comment, eocd->commentLength);
		eocd->comment[eocd->commentLength] = 0;
	}
	return eocd;
}

CentralDirectoryFileHeader* read_cdfh(std::ifstream& in) {
	uint32_t signature = 0;
	in.read((char *)&signature, sizeof(signature));
	if (signature != valid_signatures.CDFH) {
		std::cerr << "ERROR: CentralDirectoryFileHeader not found!\n";
		return nullptr;
	}
	CentralDirectoryFileHeader *cdfh = new CentralDirectoryFileHeader;
	in.read((char *)cdfh, sizeof(CentralDirectoryFileHeader) - 3 * sizeof(uint8_t *));
	if (cdfh->filenameLength) {
		cdfh->filename = new uint8_t[cdfh->filenameLength + 1];
		in.read((char *)cdfh->filename, cdfh->filenameLength);
		cdfh->filename[cdfh->filenameLength] = 0;
	}
	if (cdfh->extraFieldLength) {
		cdfh->extraField = new uint8_t[cdfh->extraFieldLength + 1];
		in.read((char *)cdfh->extraField, cdfh->extraFieldLength);
		cdfh->extraField[cdfh->extraFieldLength] = 0;
	}
	if (cdfh->fileCommentLength) {
		cdfh->fileComment = new uint8_t[cdfh->fileCommentLength + 1];
		in.read((char *)cdfh->fileComment, cdfh->fileCommentLength);
		cdfh->fileComment[cdfh->fileCommentLength] = 0;
	}
	return cdfh;
} 

DataDescriptor* read_dd(std::ifstream& in) {
	uint32_t signature = 0;
	in.read((char *)&signature, sizeof(signature));
	if (signature != valid_signatures.DD) {
		std::cerr << "ERROR: DataDescriptor not found!\n";
		return nullptr;
	}
	DataDescriptor *dd = new DataDescriptor;
	in.read((char *)dd, sizeof(DataDescriptor));
	return dd;
}

LocalFileHeader* read_lfh(std::ifstream& in) {
	uint32_t signature = 0;
	in.read((char *)&signature, sizeof(signature));
	if (signature != valid_signatures.LFH) {
		std::cerr << "ERROR: LocalFileHeader not found!\n";
		return nullptr;
	}
	LocalFileHeader *lfh = new LocalFileHeader;
	in.read((char *)lfh, sizeof(LocalFileHeader) - 2 * sizeof(uint8_t *));
	if (lfh->filenameLength) {
		lfh->filename = new uint8_t[lfh->filenameLength + 1];
		in.read((char *)lfh->filename, lfh->filenameLength);
		lfh->filename[lfh->filenameLength] = 0;
	}
	if (lfh->extraFieldLength) {
		lfh->extraField = new uint8_t[lfh->extraFieldLength + 1];
		in.read((char *)lfh->extraField, lfh->extraFieldLength);
		lfh->extraField[lfh->extraFieldLength] = 0;
	}
	return lfh;	
}

uint8_t* read_data(std::ifstream& in, const LocalFileHeader& lfh) {
	uint8_t *read_buf = new uint8_t[lfh.compressedSize];
	in.read((char *)read_buf, lfh.compressedSize);
	if (Z_DEFLATED == lfh.compressionMethod) {
		uint8_t *result = new uint8_t[lfh.uncompressedSize];
		z_stream zs;
		std::memset(&zs, 0, sizeof(zs));
		inflateInit2(&zs, -MAX_WBITS);
		zs.avail_in = lfh.compressedSize;
		zs.next_in = read_buf;
		zs.avail_out = lfh.uncompressedSize;
		zs.next_out = result;
		inflate(&zs, Z_FINISH);
		inflateEnd(&zs);
		delete [] read_buf;
		return result;
	}
	return read_buf;
}
