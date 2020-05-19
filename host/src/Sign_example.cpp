#include "ZIP_file_signing.h"
#include <cassert>
#include <vector>
#include <iostream>
#include <ostream>
#include <cstring>
#include <fstream>


int main(int argc, char *argv[]) {
    std::string certificate = "-----BEGIN CERTIFICATE-----\nMIIEHjCCAwagAwIBAgIJAOgOBeOmBds3MA0GCSqGSIb3DQEBCwUAMIGjMQswCQYD\nVQQGEwJSVTEaMBgGA1UECAwRU2FpbnQgUGV0ZXJzYm91cmcxGjAYBgNVBAcMEVNh\naW50IFBldGVyc2JvdXJnMQ4wDAYDVQQKDAVQZXR5YTELMAkGA1UECwwCSVQxFTAT\nBgNVBAMMDEl2YW4gRm9tZW5rbzEoMCYGCSqGSIb3DQEJARYZaXZhbml2YW5mb21l\nbmtvQGdtYWlsLmNvbTAeFw0yMDAzMDIxMDUzNTBaFw0yMTAzMDIxMDUzNTBaMIGj\nMQswCQYDVQQGEwJSVTEaMBgGA1UECAwRU2FpbnQgUGV0ZXJzYm91cmcxGjAYBgNV\nBAcMEVNhaW50IFBldGVyc2JvdXJnMQ4wDAYDVQQKDAVQZXR5YTELMAkGA1UECwwC\nSVQxFTATBgNVBAMMDEl2YW4gRm9tZW5rbzEoMCYGCSqGSIb3DQEJARYZaXZhbml2\nYW5mb21lbmtvQGdtYWlsLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoC\nggEBAL4Q5hiLd3+5sjhyrv89vbtfdjS5Eu5E6TPyh0bJSaIb5OUSBSG/IsP+sK0M\nCEOChwZ5E+41mVCOP6VZ8dzQnTdRPMxBb1TOWe69IPBlxs7TkJTaQHF+6lYZVgqE\noeeLrLVTY/zKZVx/EhktkXOMhJgd8mYodnwZx5QpsoaJx9RaNTIq3wbwJt1J/PAG\nei87h7F0doMw4gGcAqkr+E3nYBMzDH/l0vW0N7Lo5850c0qOEsYniufQuT3NCaoh\nTsiknPigO27/8wDOAHjwrbMsJrUsvLxOgYRLuqT5hvnb4pCYsCRqHw14jTqG3c6B\nkBIASSw5amS8PpiTCRAD6fNiUzcCAwEAAaNTMFEwHQYDVR0OBBYEFFmR0SPw4AZt\nCLNgjlwZ350gfIEyMB8GA1UdIwQYMBaAFFmR0SPw4AZtCLNgjlwZ350gfIEyMA8G\nA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBADWAM4y9BRf+gPQB19R0\nK9OxMv+rxUcq0PxfBNOPWRmkMPXY0ayEHWEUr0DP5t1kCCX+qixDEuuI1+7T2VpV\nTTKGrfBrb51iF0390A81MI1IKX8y1HnMRZQ65Y+ivZXCiys+UPOqnP1zuq4fEBo1\nSRTEVxkI8uwc24lTigP0kuG/fJ7fxEk9x6nyd0TbxKpN1Ndjc+C5Y9TgC8Xt1tAm\n0d7UZ/O3LXuQmGiw28iwPHHEHZa1XOirKuXzctrx8x7TlXphagPlKlp2UCMn607N\nMsleqT2UPuUarUHc7qgz+GTJlQj7o2qF8KrLxSPOk5mGXjQhSRTk7beX/XLLM7Zk\nfAs=\n-----END CERTIFICATE-----";
	using ZIP_file_signing::ZIP_file;
	if (argc > 1) {
		ZIP_file z(argv[1]);
		std::cout << z.is_64() << '\n';
		z.load_certificate_and_signing(certificate);
		std::string res = z.get_certificate_by_string();
		std::cout << (res == certificate) << '\n';
		std::cout << z.check_sign() << '\n';
	}
	return 0;
}
