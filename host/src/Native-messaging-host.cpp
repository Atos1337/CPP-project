#include <iostream>
#include <string>
#include <optional>
#include <json.hpp>
#include "portable-file-dialogs.h"
#include "ZIP_file_signing.h"
#include "zipSigner.hpp"

using json = nlohmann::json;

namespace message {

json getMessage() {
    size_t length = 0;
    for (size_t i = 0; i < 4; i++) {
        unsigned char read_char = getchar();
        length = length | (read_char << i * 8);
    }

    std::string msg = "";
    for (size_t i = 0; i < length; i++) {
        msg += getchar();
    }
    return json::parse(msg);
}

void sendMessage(const json &json_msg) {
    std::string msg = json_msg.dump();
    size_t len = msg.length();
    for (size_t i = 0; i <= 24; i += 8) {
        std::cout << char(len >> i);
    }
    std::cout << msg << std::flush;
}

} // namespace message

namespace {

std::optional<std::string> openZip() {
    auto f = pfd::open_file("Choose files to read", "",
                        { "Zip Files (.zip)", "*.zip",
                          "All Files", "*" },
                        true);
    if (f.result().empty()) {
        return std::nullopt;
    }
    return f.result()[0];
}

std::vector<std::string> bytesToNames(const std::vector<std::vector<uint8_t>> &v) {
    std::vector<std::string> res;
    for (auto &i : v) {
        res.push_back(std::string(i.cbegin(), i.cend() - 1));
    }
    return res;
}

}

namespace certificate {

json check(const json &msg) {
    std::optional<std::string> filepath = std::nullopt;
    if (msg["request"] == "Check certificate in file") {
        if (msg.find("filepath") != msg.end()) {
            filepath = msg["filepath"];
        }
    }
    else {
        filepath = openZip();
    }
    json j;
    if (!filepath) {
        j["Error"] = "No file found";
    }
    else {
        j["ArchiveName"] = *filepath;
        ZipSigner zipSigner;
        ZIP_file_signing::ZIP_file zip_file(filepath->c_str(), zipSigner);
        j["Verified"] = zip_file.check_sign() ? "OK" : "FAILED";
        j["ArchiveFiles"] = bytesToNames(zip_file.get_filenames());
        if (j["Verified"] == "OK") {
            j["Certificate"] = zip_file.get_certificate_by_string();
            j["CertificateData"] = zip_file.getCertificateData();
        }
    }
    return j;
}

json sign(const json &msg) {
    std::optional<std::string> filepath = openZip();
    json j;
    if (!filepath) {
        j["Error"] = "No file found";
        
    }
    else if (msg.find("privateKey") == msg.end() || msg["privateKey"].size() == 0) {
        j["Error"] = "No private key found";
    }
    else {
        j["ArchiveName"] = *filepath;
        ZipSigner zipSigner(msg["privateKey"]);
        ZIP_file_signing::ZIP_file zip_file((*filepath).c_str(), zipSigner);
        std::cerr << msg["certificate"] << std::endl;
        zip_file.load_certificate_and_signing(msg["certificate"]);

        j["ArchiveFiles"] = bytesToNames(zip_file.get_filenames());
        j["Create sign"] = "OK";
    }
    return j;
}

} // namespace certificate

int main(){
    while (true) {
        json msg = message::getMessage();
        //std::cerr << msg.dump() << std::endl;
        json res;
        if (msg["request"] == "Check certificate in file" ||
                msg["request"] == "Open and check certificate") {

            res = certificate::check(msg);
        }
        else if (msg["request"] == "Open and sign certificate") {
            res = certificate::sign(msg);
        }
        message::sendMessage(res);
    }
    return 0;
}