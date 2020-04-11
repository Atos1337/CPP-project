#include "ZIP_file_signing.h"
#include "ZIP_file_reading.h"
#include "ZIP_file_writing.h"
#include "Test_function_for_sign.h"

#include <iostream>
#include <experimental/filesystem>
#include <cassert>

namespace ZIP_file_signing {
	using namespace ZIP_file_reading;
	using namespace ZIP_file_writing;

	ZIP_file::ZIP_file(const char *filename) : arch(filename) {}

	void ZIP_file::signing() {
		std::ifstream in(arch.c_str(), std::ios::binary);
		std::ofstream out("tmp.zip", std::ios::binary);
		EOCD eocd;
		in >> eocd;
		std::vector<uint32_t> lfh_offsets(eocd.totalCentralDirectoryRecord);
		in.seekg(eocd.centralDirectoryOffset, in.beg);
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			CentralDirectoryFileHeader cdfh;
			in >> cdfh;
			lfh_offsets[i] = cdfh.localFileHeaderOffset;
		}
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			in.seekg(lfh_offsets[i], in.beg);
			lfh_offsets[i] = out.tellp();
			LocalFileHeader lfh;
			File f;
			in >> lfh;
			std::unique_ptr<uint8_t[]> tmp1(new uint8_t[lfh.compressedSize]);
			in.read(reinterpret_cast<char *>(tmp1.get()), lfh.compressedSize);
			f = {std::unique_ptr<uint8_t[]>(new uint8_t[lfh.compressedSize]), lfh.compressionMethod, lfh.compressedSize, lfh.uncompressedSize};
			memcpy(f.data.get(), tmp1.get(), lfh.compressedSize);
			inflate_data(f);
			std::vector<uint8_t> sign = test_sign(f);
			f.data = std::move(tmp1);
			extraFieldRecord efr = {0x0015, static_cast<uint16_t>(sign.size()), std::unique_ptr<uint8_t[]>(new uint8_t[sign.size()])};
			memcpy(efr.data.get(), sign.data(), sign.size());
			lfh.extraFieldLength += 2 * sizeof(uint16_t) + efr.size;
			std::unique_ptr<extraFieldRecord[]> tmp = std::make_unique<extraFieldRecord[]>(lfh.totalExtraFieldRecord + 1);
			for (int j = 0; j < lfh.totalExtraFieldRecord; ++j) {
				tmp[j] = std::move(lfh.extraField[j]);
			}
			tmp[lfh.totalExtraFieldRecord] = std::move(efr);
			++lfh.totalExtraFieldRecord;
			lfh.extraField = std::move(tmp);
			out << lfh << f;
		}
		in.seekg(eocd.centralDirectoryOffset, in.beg);
		eocd.centralDirectoryOffset = out.tellp();
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			CentralDirectoryFileHeader cdfh;
			in >> cdfh;
			cdfh.localFileHeaderOffset = lfh_offsets[i];
			out << cdfh;
		}
		in.close();
		std::experimental::filesystem::remove(arch.c_str());
		out << eocd;
		out.close();
		std::experimental::filesystem::rename("tmp.zip", arch.c_str());
	}

	bool ZIP_file::check_sign() {
		bool is_correct = true;
		std::ifstream in(arch.c_str(), std::ios::binary);
		EOCD eocd;
		in >> eocd;
		std::vector<uint32_t> lfh_offsets(eocd.totalCentralDirectoryRecord);
		in.seekg(eocd.centralDirectoryOffset, in.beg);
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			CentralDirectoryFileHeader cdfh;
			in >> cdfh;
			lfh_offsets[i] = cdfh.localFileHeaderOffset;
		}
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			in.seekg(lfh_offsets[i], in.beg);
			LocalFileHeader lfh;
			File f;
			in >> lfh;
			std::unique_ptr<uint8_t[]> tmp1(new uint8_t[lfh.compressedSize]);
			in.read(reinterpret_cast<char *>(tmp1.get()), lfh.compressedSize);
			f = {std::unique_ptr<uint8_t[]>(new uint8_t[lfh.compressedSize]), lfh.compressionMethod, lfh.compressedSize, lfh.uncompressedSize};
			memcpy(f.data.get(), tmp1.get(), lfh.compressedSize);
			inflate_data(f);
			std::vector<uint8_t> sign;
			for (int j = 0; j < lfh.totalExtraFieldRecord; ++j) {
				auto &efr = lfh.extraField[j];
				if (efr.signature == 0x0015) {
					sign.resize(efr.size);
					memcpy(sign.data(), efr.data.get(), efr.size);
					break;
				}
			}
			is_correct &= ch_sign(f, sign);
		}
		return is_correct;
	}

	void ZIP_file::load_certificate(const char *certificate) {
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
		for (int i = 0; i < eocd->totalCentralDirectoryRecord; ++i) {
			if (i == 0) {
				uint8_t *buf = nullptr;
				uint16_t size = serialize(certificate, buf);
				cdfh[i].extraFieldLength += 2 * sizeof(uint16_t) + size;;
				eocd->sizeOfCentralDirectory += 2 * sizeof(uint16_t) + size;
				extraFieldRecord efr = {0x0014, size, std::make_unique<uint8_t[]>(size)};
				memcpy(efr.data.get(), buf, size);
				std::unique_ptr<extraFieldRecord[]> tmp = std::make_unique<extraFieldRecord[]>(cdfh[i].totalExtraFieldRecord + 1);
				for (int j = 0; j < cdfh[i].totalExtraFieldRecord; ++j) {
					tmp[j] = std::move(cdfh[i].extraField[j]);
				}
				tmp[cdfh[i].totalExtraFieldRecord] = std::move(efr);
				++cdfh[i].totalExtraFieldRecord;
				cdfh[i].extraField = std::move(tmp);
			}
			out << cdfh[i];
		}
		out << *eocd;
	}

	/*std::vector<uint8_t> ZIP_file::get_certificate(const CentralDirectoryFileHeader &cdfh) {
		std::vector<uint8_t> certificate;
		for (auto &efr : cdfh.extraField) {
			if (efr.signature == 0x0014) {
				certificate = efr.data;
				break;
			}
		}
		return std::move(certificate);
	}

	std::vector<uint8_t> ZIP_file::get_signature(const LocalFileHeader &lfh) {
		std::vector<uint8_t> signature;
		for (auto &efr : lfh.extraField) {
			if (efr.signature == 0x0015) {
				signature = efr.data;
				break;
			}
		}
		return std::move(signature);
	}*/
} //namespace ZIP_file_signing
