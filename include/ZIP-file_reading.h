#pragma once

#include <fstream>

#include "ZIP-file_headers.h"
#include "zlib.h"

namespace ZIP_file_reading {

std::ifstream& operator>>(std::ifstream& in, EOCD& eocd);

std::ifstream& operator>>(std::ifstream& in, CentralDirectoryFileHeader& cdfh);

std::ifstream& operator>>(std::ifstream& in, DataDescriptor& dd);

std::ifstream& operator>>(std::ifstream& in, LocalFileHeader& lfh);

void inflate_data(File& f);

std::ifstream& operator>>(std::ifstream& in, File& f);

std::ifstream& operator>>(std::ifstream& in, std::ofstream& out);

} //namespace ZIP_file_reading