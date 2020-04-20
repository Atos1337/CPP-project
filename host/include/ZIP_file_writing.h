#pragma once
#pragma pack(1)

#include "ZIP_file_headers.h"

#include <cstring>
#include <cstdint>
#include <fstream>

namespace ZIP_file_writing{

std::ofstream& operator<<(std::ofstream& out, EOCD& eocd);

std::ofstream& operator<<(std::ofstream& out, CentralDirectoryFileHeader& cdfh);

std::ofstream& operator<<(std::ofstream& out, DataDescriptor& dd);

std::ofstream& operator<<(std::ofstream& out, LocalFileHeader& lfh);

std::ofstream& operator<<(std::ofstream& out, File& f);

uint16_t serialize(std::string& certificate, uint8_t *&buf);

} //namespace ZIP_file_writing

