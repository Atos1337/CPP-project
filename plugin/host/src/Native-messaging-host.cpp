#include <iostream>
#include <string>
#include <json.hpp>
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

int main(){
    while (true) {
        json msg = message::getMessage();
        std::cerr << msg.dump() << std::endl;
        if (msg.find("file") != msg.end()) {
            message::sendMessage(R"({"Certificate": "123"})");
        }
        else {
            if (msg["text"] == "Hi") {
                message::sendMessage(R"({"text": "Hi from C++"})");
            }
            else {
                message::sendMessage(R"({"text": "Unknown command"})");
            }
        }
    }

    return 0;
}