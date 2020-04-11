#pragma once
#include <vector>
#include "ZIP_file_headers.h"

std::vector<uint8_t> test_sign(File &f);

bool ch_sign(File &f, const std::vector<uint8_t> &sign);
