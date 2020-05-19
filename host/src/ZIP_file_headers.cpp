#include "ZIP_file_headers.h"

template<typename T>
T vector_to_integral(const std::vector<uint8_t>& data, int i) {
	T res = 0;
	for (size_t j = 0; j < sizeof(T); ++j, ++i) {
		res |= static_cast<T>(data[i]) << (8 * j);
	}
	return res;
}

template<typename T>
void integral_to_vector(std::vector<uint8_t> &data, int i, T val) {
	for (size_t j = 0; j < sizeof(T); ++i, ++j) {
		uint8_t tmp = (val >> (8 * j)) & 255;
		data[i] = tmp;
	}
}

bool EOCD::is_64() const {
	return diskNumber == 0xFFFF || startDiskNumber == 0xFFFF || numberCentralDirectoryRecord == 0xFFFF || totalCentralDirectoryRecord == 0xFFFF || sizeOfCentralDirectory == 0xFFFFFFFF || centralDirectoryOffset == 0xFFFFFFFF || commentLength == 0xFFFF;
} 

bool CentralDirectoryFileHeader::is_64() const {
	return compressedSize == 0xFFFFFFFF || uncompressedSize == 0xFFFFFFFF || localFileHeaderOffset == 0xFFFFFFFF || diskNumber == 0xFFFF;
}

bool LocalFileHeader::is_64() const {
	return compressedSize == 0xFFFFFFFF || uncompressedSize == 0xFFFFFFFF;
}

ZIP64EI::ZIP64EI(const std::vector<uint8_t> &data, const CentralDirectoryFileHeader &cdfh) {
	int j = 0;
	if (cdfh.uncompressedSize == 0xFFFFFFFF)
		uncompressedSize = vector_to_integral<uint64_t>(data, (8 * j++));
	if (cdfh.compressedSize == 0xFFFFFFFF)
		compressedSize = vector_to_integral<uint64_t>(data, (8 * j++));
	if (cdfh.localFileHeaderOffset == 0xFFFFFFFF) {
		localFileHeaderOffset = vector_to_integral<uint64_t>(data, (8 * j++));
	}
	if (cdfh.diskNumber == 0xFFFF) {
		diskNumber = vector_to_integral<uint32_t>(data, (8 * j++));
	}
}

ZIP64EI::ZIP64EI(const std::vector<uint8_t> &data, const LocalFileHeader &cdfh) {
	int j = 0;
	if (cdfh.uncompressedSize == 0xFFFFFFFF)
		uncompressedSize = vector_to_integral<uint64_t>(data, (8 * j++));
	if (cdfh.compressedSize == 0xFFFFFFFF)
		compressedSize = vector_to_integral<uint64_t>(data, (8 * j++));
}

int ZIP64EI::to_vector(std::vector<uint8_t> &res, CentralDirectoryFileHeader &cdfh) {
	int j = 0;
	int size = 0;
	if (cdfh.uncompressedSize == 0xFFFFFFFF) {
		res.resize(8 * (j + 1));
		integral_to_vector(res, 8 * j++, uncompressedSize);
		size += 8;
	}
	if (cdfh.compressedSize == 0xFFFFFFFF) {
		res.resize(8 * (j + 1));
		integral_to_vector(res, 8 * j++, compressedSize);
		size += 8;
	}
	if (cdfh.localFileHeaderOffset == 0xFFFFFFFF) {
		res.resize(8 * (j + 1));
		integral_to_vector(res, 8 * j++, localFileHeaderOffset);
		size += 8;
	}
	if (cdfh.diskNumber == 0xFFFF) {
		res.resize(8 * j + 4);
		integral_to_vector(res, 8 * j++, diskNumber);
		size += 4;
	}
	return size;
}

int ZIP64EI::to_vector(std::vector<uint8_t> &res, LocalFileHeader &cdfh) {
	int j = 0;
	int size = 0;
	if (cdfh.uncompressedSize == 0xFFFFFFFF) {
		res.resize(8 * (j + 1));
		integral_to_vector(res, 8 * j++, uncompressedSize);
		size += 8;
	}
	if (cdfh.compressedSize == 0xFFFFFFFF) {
		res.resize(8 * (j + 1));
		integral_to_vector(res, 8 * j++, compressedSize);
		size += 8;
	}
	return size;
}

EOCD64::EOCD64(const EOCD &eocd) : eocd64Size(sizeof(EOCD64) - 8 - sizeof(std::vector<uint8_t>)), versionMadeBy(1), versionToExtract(1), diskNumber(eocd.diskNumber), startDiskNumber(eocd.startDiskNumber), numberCentralDirectoryRecord(eocd.numberCentralDirectoryRecord), totalCentralDirectoryRecord(eocd.totalCentralDirectoryRecord), sizeOfCentralDirectory(eocd.sizeOfCentralDirectory), centralDirectoryOffset(eocd.centralDirectoryOffset), data_sector(std::vector<uint8_t>(0)) {}

extraFieldRecord& CentralDirectoryFileHeader::get_efr_of_ei() {
	for (size_t i = 0; i < extraField.size(); ++i) {
		if (extraField[i].signature == 0x0001) {
			return extraField[i];
		}
	}
	return extraField[0];
}
