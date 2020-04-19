#include "ZIP_file_signing.h"
#include "ZIP_file_reading.h"
#include "ZIP_file_writing.h"
#include "Test_function_for_sign.h"
#include "openssl/x509.h"

#include <iostream>
#include <experimental/filesystem>
#include <cassert>

namespace ZIP_file_signing {
	using namespace ZIP_file_reading;
	using namespace ZIP_file_writing;

	ZIP_file::ZIP_file(const char *filename, ZipSigner &signer_) : arch(filename), signer(signer_) {}

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
			std::vector<uint8_t> tmp(lfh.compressedSize);
			in.read(reinterpret_cast<char *>(tmp.data()), lfh.compressedSize);
			f = {tmp, lfh.compressionMethod, lfh.compressedSize, lfh.uncompressedSize};
			inflate_data(f);
			std::vector<uint8_t> sign = signer.sign(f.data);
			f.data = std::move(tmp);
			extraFieldRecord efr = {0x0015, static_cast<uint16_t>(sign.size()), std::move(sign)};
			lfh.extraFieldLength += 2 * sizeof(uint16_t) + efr.size;
			lfh.extraField.push_back(std::move(efr));
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
		using X509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;
		bool is_correct = true;
		std::ifstream in(arch.c_str(), std::ios::binary);
		EOCD eocd;
		in >> eocd;
		std::vector<uint32_t> lfh_offsets(eocd.totalCentralDirectoryRecord);
		in.seekg(eocd.centralDirectoryOffset, in.beg);
		X509_ptr certificate(nullptr, X509_free);
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			CentralDirectoryFileHeader cdfh;
			in >> cdfh;
			if (i == 0) {
				certificate = std::move(get_certificate(cdfh));
			}
			lfh_offsets[i] = cdfh.localFileHeaderOffset;
		}
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			in.seekg(lfh_offsets[i], in.beg);
			LocalFileHeader lfh;
			File f;
			in >> lfh;
			f = {std::vector<uint8_t>(0), lfh.compressionMethod, lfh.compressedSize, lfh.uncompressedSize};
			in >> f;
			inflate_data(f);
			std::vector<uint8_t> sign = get_signature(lfh);
			is_correct &= signer.verify(certificate.get(), f.data, sign);
		}
		return is_correct;
	}

	void ZIP_file::load_certificate(const char *certificate) {
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
			f = {std::vector<uint8_t>(0), lfh.compressionMethod, lfh.compressedSize, lfh.uncompressedSize};
			in >> f;
			out << lfh << f;
		}
		eocd.centralDirectoryOffset = out.tellp();
		for (int i = 0; i < eocd.totalCentralDirectoryRecord; ++i) {
			CentralDirectoryFileHeader cdfh;
			in >> cdfh;
			if (i == 0) {
				uint8_t *buf = nullptr;
				uint16_t size = serialize(certificate, buf);
				cdfh.extraFieldLength += 2 * sizeof(uint16_t) + size;;
				eocd.sizeOfCentralDirectory += 2 * sizeof(uint16_t) + size;
				extraFieldRecord efr = {0x0014, size, std::vector<uint8_t>(size)};
				memcpy(efr.data.data(), buf, size);
				cdfh.extraField.push_back(std::move(efr));
			}
			out << cdfh;
		}
		out << eocd;
		in.close();
		std::experimental::filesystem::remove(arch.c_str());
		out << eocd;
		out.close();
		std::experimental::filesystem::rename("tmp.zip", arch.c_str());
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
