#pragma once

#include <fstream>

#include "ZIP-file_headers.h"
#include "zlib.h"

EOCD* read_eocd(std::ifstream& in);

CentralDirectoryFileHeader* read_cdfh(std::ifstream& in);

DataDescriptor* read_dd(std::ifstream& in);

LocalFileHeader* read_lfh(std::ifstream& in);

uint8_t* read_data(std::ifstream& in, const LocalFileHeader& lfh);