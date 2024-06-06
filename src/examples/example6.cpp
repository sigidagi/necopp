#include "necopp.hpp"
#include <iostream>
#include <unistd.h>

using namespace std::chrono_literals;

int main_(int, char **) {

    int fd = neco::dial("tcp", "example.com:80");
    if (fd < 0) {
        std::cerr << "Failed to dial" << std::endl;
        return 1;
    }

    std::string req = 
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Connection: close\r\n"
        "\r\n";
    
    neco::io client{fd};
    // Send request
    client.write(req);
    // Read response
    auto output = client.read(4096);
    // Print response
    std::cout << std::string(output.data(), output.size()) << std::endl;
    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    neco::run(argc, argv, main_);
}
