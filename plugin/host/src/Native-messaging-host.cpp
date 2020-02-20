#include <iostream>
#include <string>

std::string getMessage() {
    size_t length = 0;
    for (size_t i = 0; i < 4; i++) {
        unsigned char read_char = getchar();
        length = length | (read_char << i * 8);
    }

    std::string msg = "";
    for (size_t i = 0; i < length; i++) {
        msg += getchar();
    }
    return msg;
}

void sendMessage(const std::string &msg) {
    size_t len = msg.length();
    for (size_t i = 0; i <= 24; i += 8) {
        std::cout << char(len >> i);
    }
    std::cout << msg << std::flush;
}

int main(){
    
    while (true) {
        std::string msg = getMessage();
        std::cerr << msg << std::endl;
        if (msg == "{\"text\":\"Hi\"}") {
            sendMessage("{\"text\":\"Hi from C++\"}");
        }
        else {
            sendMessage("{\"text\":\"Unknown command\"}");
        }
    }

    return 0;
}