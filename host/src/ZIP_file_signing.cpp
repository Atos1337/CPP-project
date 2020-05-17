#include "ZIP_file_signing.h"
#include "ZIP_file_reading.h"
#include "ZIP_file_writing.h"
#include "openssl/x509.h"

#include <iostream>
#include <experimental/filesystem>
#include <cassert>
#include <algorithm>

namespace ZIP_file_signing {
	using namespace ZIP_file_reading;
	using namespace ZIP_file_writing;

	ZIP_file::ZIP_file(const char *filename, ZipSigner &signer_) : arch(filename), signer(signer_) {}

	bool ZIP_file::is_64() {
		std::ifstream in(arch.c_str(), std::ios::binary);
		EOCD eocd;
		in >> eocd;
		return eocd.is_64();
	}

	std::string	ZIP_file::get_result_name(std::string& arch) {
		std::reverse(arch.begin(), arch.end());
		size_t found = arch.find("/");
		std::reverse(arch.begin(), arch.end());
		if (found != std::string::npos)
			return arch.substr(0, arch.length() - found);
		else
			return "";
	}

	std::vector<std::vector<uint8_t>> ZIP_file::get_filenames() {
		std::ifstream in(arch.c_str(), std::ios::binary);
		EOCD eocd;
		in >> eocd;
		std::vector<std::vector<uint8_t>> filenames;
		in.seekg(eocd.centralDirectoryOffset, in.beg);
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			CentralDirectoryFileHeader cdfh;
			in >> cdfh;
			filenames.push_back(cdfh.filename);
		}
		return std::move(filenames);
	}

	bool ZIP_file::check_sign() {
		using X509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;
		bool is_correct = true;
		std::ifstream in(arch.c_str(), std::ios::binary);
		EOCD eocd;
		EOCD64Locator eocdl;
		EOCD64 eocd64;
		in >> eocd;
		uint64_t centralDirectoryOffset = eocd.centralDirectoryOffset;
		uint64_t totalCentralDirectoryRecord = eocd.totalCentralDirectoryRecord;
		if (eocd.is_64()) {
			in >> eocdl;
			in.seekg(eocdl.eocd64Offset, in.beg);
			in >> eocd64;
			centralDirectoryOffset = eocd64.centralDirectoryOffset;
			totalCentralDirectoryRecord = eocd64.totalCentralDirectoryRecord;
		}
		std::vector<uint32_t> lfh_offsets(totalCentralDirectoryRecord);
		in.seekg(centralDirectoryOffset, in.beg);
		X509_ptr certificate(nullptr, X509_free);
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			CentralDirectoryFileHeader cdfh;
			in >> cdfh;
			if (i == 0) {
				certificate = std::move(get_certificate(cdfh));
			}
			lfh_offsets[i] = cdfh.localFileHeaderOffset;
			if (cdfh.is_64() && cdfh.localFileHeaderOffset == 0xFFFFFFFF) {
				lfh_offsets[i] = ZIP64EI(get_ei(cdfh)).localFileHeaderOffset;
			}
		}
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			in.seekg(lfh_offsets[i], in.beg);
			LocalFileHeader lfh;
			File f;
			in >> lfh;
			uint64_t compressedSize = lfh.compressedSize;
			uint64_t uncompressedSize = lfh.uncompressedSize;
			ZIP64EI ei;
			if (lfh.is_64()) {
				ei = ZIP64EI(get_ei(lfh));
				if (lfh.compressedSize == 0xFFFFFFFF)
					compressedSize = ei.compressedSize;
				if (lfh.uncompressedSize == 0xFFFFFFFF)
					uncompressedSize = ei.uncompressedSize;
			}
			f = {std::vector<uint8_t>(0), lfh.compressionMethod, compressedSize, uncompressedSize};
			in >> f;
			inflate_data(f);
			std::vector<uint8_t> sign = get_signature(lfh);
			is_correct &= signer.verify(certificate.get(), f.data, sign);
            //is_correct &= ch_sign(certificate.get(), f.data, sign);
		}
		return is_correct;
	}

	void ZIP_file::load_certificate_and_signing(const std::string& certificate) {
		std::ifstream in(arch.c_str(), std::ios::binary);
		std::string test = get_result_name(arch);
		test += "result.zip";
		std::ofstream out(test.c_str(), std::ios::binary);
		EOCD eocd;
		EOCD64Locator eocdl;
		EOCD64 eocd64;
		in >> eocd;
		uint64_t centralDirectoryOffset = eocd.centralDirectoryOffset;
		uint64_t totalCentralDirectoryRecord = eocd.totalCentralDirectoryRecord;
		if (eocd.is_64()) {
			in >> eocdl;
			in.seekg(eocdl.eocd64Offset, in.beg);
			in >> eocd64;
			centralDirectoryOffset = eocd64.centralDirectoryOffset;
			totalCentralDirectoryRecord = eocd64.totalCentralDirectoryRecord;
		}
		std::vector<uint64_t> lfh_offsets(totalCentralDirectoryRecord);
		in.seekg(centralDirectoryOffset, in.beg);
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			CentralDirectoryFileHeader cdfh;
			in >> cdfh;
			lfh_offsets[i] = cdfh.localFileHeaderOffset;
			if (cdfh.is_64() && cdfh.localFileHeaderOffset == 0xFFFFFFFF) {
				lfh_offsets[i] = ZIP64EI(get_ei(cdfh)).localFileHeaderOffset;
			}
		}
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			in.seekg(lfh_offsets[i], in.beg);
			lfh_offsets[i] = out.tellp();
			LocalFileHeader lfh;
			File f;
			in >> lfh;
			uint64_t compressedSize = lfh.compressedSize;
			uint64_t uncompressedSize = lfh.uncompressedSize;
			ZIP64EI ei;
			if (lfh.is_64()) {
				ei = ZIP64EI(get_ei(lfh));
				if (lfh.compressedSize == 0xFFFFFFFF)
					compressedSize = ei.compressedSize;
				if (lfh.uncompressedSize == 0xFFFFFFFF)
					uncompressedSize = ei.uncompressedSize;
			}
			std::vector<uint8_t> tmp(compressedSize);
			in.read(reinterpret_cast<char *>(tmp.data()), compressedSize);
			f = {tmp, lfh.compressionMethod, compressedSize, uncompressedSize};
			inflate_data(f);
			std::vector<uint8_t> sign = signer.sign(f.data);
			f.data = std::move(tmp);
			extraFieldRecord efr = {0x0015, static_cast<uint16_t>(sign.size()), std::move(sign)};
			lfh.extraFieldLength += 2 * sizeof(uint16_t) + efr.size;
			lfh.extraField.push_back(std::move(efr));
			out << lfh << f;
		}
		in.seekg(centralDirectoryOffset, in.beg);
		centralDirectoryOffset = out.tellp();
		uint64_t sizeOfCentralDirectory = out.tellp();
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			CentralDirectoryFileHeader cdfh;
			in >> cdfh;
			if (i == 0) {
				uint8_t *buf = nullptr;
				uint16_t size = serialize(certificate, buf);
				cdfh.extraFieldLength += 2 * sizeof(uint16_t) + size;
				extraFieldRecord efr = {0x0014, size, std::vector<uint8_t>(size)};
				memcpy(efr.data.data(), buf, size);
				cdfh.extraField.push_back(std::move(efr));
			}
			cdfh.localFileHeaderOffset = lfh_offsets[i];
			if (!cdfh.is_64() && lfh_offsets[i] >= 0xFFFFFFFF) {
				ZIP64EI ei;
				ei.uncompressedSize = cdfh.uncompressedSize;
				ei.compressedSize = cdfh.compressedSize;
				ei.localFileHeaderOffset = lfh_offsets[i];
				ei.diskNumber = cdfh.diskNumber;
				extraFieldRecord efr = {0x0001, 28, std::vector<uint8_t>(28)};
				ei.to_vector(efr.data);
				cdfh.extraField.push_back(std::move(efr));
				cdfh.localFileHeaderOffset = 0xFFFFFFFF;
			}
			if (cdfh.is_64() && lfh_offsets[i] >= 0xFFFFFFFF) {
				std::vector<uint8_t> &data = get_ei(cdfh);
				ZIP64EI ei(data);
				ei.localFileHeaderOffset = lfh_offsets[i];
				cdfh.localFileHeaderOffset = 0xFFFFFFFF;
				ei.to_vector(data);
			}
			out << cdfh;
		}
		uint64_t eocd64Offset = out.tellp();
		sizeOfCentralDirectory = eocd64Offset - sizeOfCentralDirectory;
		if (eocd.is_64()) {
			eocdl.eocd64Offset = eocd64Offset + sizeof(eocdl) + sizeof(eocd) + 2 * sizeof(uint32_t);
			eocd64.sizeOfCentralDirectory = sizeOfCentralDirectory;
			eocd64.centralDirectoryOffset = centralDirectoryOffset;
			out << eocdl << eocd << eocd64;
		} else if (sizeOfCentralDirectory >= 0xFFFFFFFF || centralDirectoryOffset >= 0xFFFFFFFF) {
			eocd64 = EOCD64(eocd);
			eocd64.sizeOfCentralDirectory = sizeOfCentralDirectory;
			eocd64.centralDirectoryOffset = centralDirectoryOffset;
			eocdl = {eocd.diskNumber, eocd64Offset + sizeof(eocdl) + sizeof(eocd) + 2 * sizeof(uint32_t), eocd.diskNumber};
			eocd.centralDirectoryOffset = eocd.sizeOfCentralDirectory = 0xFFFFFFFF;
			out << eocdl << eocd << eocd64;
		} else {
			eocd.centralDirectoryOffset = centralDirectoryOffset;
			eocd.sizeOfCentralDirectory = sizeOfCentralDirectory;
			out << eocd;
		}
		in.close();
		out.close();
		arch = test;
	}

	std::string ZIP_file::get_certificate_by_string() {
        std::ifstream in(arch.c_str(), std::ios::binary);
        EOCD eocd;
        in >> eocd;
        in.seekg(eocd.centralDirectoryOffset, in.beg);
        CentralDirectoryFileHeader cdfh;
        in >> cdfh;
        return get_certificate_to_check(get_certificate(cdfh));
	} 

	auto ZIP_file::get_certificate(const CentralDirectoryFileHeader &cdfh) -> X509_ptr {
		std::vector<uint8_t> certificate;
		for (auto &efr : cdfh.extraField) {
			if (efr.signature == 0x0014) {
				certificate = efr.data;
				break;
			}
		}
		return std::move(deserialize(certificate));
	}

	std::vector<uint8_t>& ZIP_file::get_ei(CentralDirectoryFileHeader &cdfh) {
		for (size_t i = 0; i < cdfh.extraField.size(); ++i) {
			if (cdfh.extraField[i].signature == 0x0001)
				return cdfh.extraField[i].data;
		}
		return cdfh.extraField[0].data;
	}

	std::vector<uint8_t>& ZIP_file::get_ei(LocalFileHeader &lfh) {
		for (size_t i = 0; i < lfh.extraField.size(); ++i) {
			if (lfh.extraField[i].signature == 0x0001)
				return lfh.extraField[i].data;
		}
		return lfh.extraField[0].data;
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
	}
} //namespace ZIP_file_signing
