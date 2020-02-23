#include "ZIP-file_reading.h"
#include <fstream>
#include <iostream>
#include <memory>

int main(int argc, char *argv[]) {
	if (argc == 1) return 0;
	std::ifstream in(argv[1], std::ios::binary);
	EOCD *eocd = read_eocd(in);
	std::cout << eocd->totalCentralDirectoryRecord << '\n';
	CentralDirectoryFileHeader **cdfh = new CentralDirectoryFileHeader *[eocd->totalCentralDirectoryRecord];
	in.seekg(eocd->centralDirectoryOffset, in.beg);
	for (int i = 0; i < eocd->totalCentralDirectoryRecord; i++) {
		cdfh[i] = read_cdfh(in);
	}
	LocalFileHeader **lfh = new LocalFileHeader *[eocd->totalCentralDirectoryRecord];
	uint8_t  **data = new uint8_t *[eocd->totalCentralDirectoryRecord];
	for (int i = 0; i < eocd->totalCentralDirectoryRecord; i++) {
		in.seekg(cdfh[i]->localFileHeaderOffset, in.beg);
		lfh[i] = read_lfh(in);
		std::cout << lfh[i]->filename << '\n';
		data[i] = read_data(in, *lfh[i]);
	}
	for (int i = 0; i < eocd->totalCentralDirectoryRecord; i++) {
		delete data[i];
		delete lfh[i];
		delete cdfh[i];
	}
	delete [] data;
	delete [] lfh;
	delete [] cdfh;
	delete eocd;
	return 0;
} 
