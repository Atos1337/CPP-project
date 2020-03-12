#include <string>

namespace ZIP_file_signing{

class ZIP_file{
public:
	ZIP_file(const char *filename);
	void signing(const char *sertificate);
	void get_and_write_sertificate(const char *sertificate);
private:
	std::string arch;
};

} //namespace ZIP_file_signing
