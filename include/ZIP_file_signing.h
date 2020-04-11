#include <string>
#include <vector>

namespace ZIP_file_signing{

class ZIP_file{
public:
	ZIP_file(const char *filename);
	void signing();
	bool check_sign();
	void load_certificate(const char *certificate);
	/*std::vector<uint8_t> get_certificate(const CentralDirectoryFileHeader &cdfh);
	std::vector<uint8_t> get_signature(const LocalFileHeader &lfh);*/
private:
	std::string arch;
};

} //namespace ZIP_file_signing
