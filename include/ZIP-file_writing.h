#pragma once
#pragma pack(1)

#include "ZIP-file_headers.h"

#include <cstring>
#include <cstdint>
#include <fstream>

namespace ZIP_file_writing{

std::ofstream& operator<<(std::ofstream& out, EOCD& eocd);

std::ofstream& operator<<(std::ofstream& out, CentralDirectoryFileHeader& cdfh);

std::ofstream& operator<<(std::ofstream& out, DataDescriptor& dd);

std::ofstream& operator<<(std::ofstream& out, LocalFileHeader& lfh);

std::ofstream& operator<<(std::ofstream& out, File& f);

std::ofstream& operator<<(std::ofstream& out, std::ifstream& in);

} //namespace ZIP_file_writing

