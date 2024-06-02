#include <unistd.h>
#include <fmt/printf.h>
#include "necopp.hpp"
#include <iostream>

int main_(int, char **) {

    int fd = neco::dial("tcp", "example.com:80");
    if (fd < 0) {
        fmt::print("neco_dial: %s\n", neco_strerror(fd));
        return 0;
    }

    std::string req = 
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Connection: close\r\n"
        "\r\n";
    
    neco::io client{fd};
    
    client.write(req);
    auto output = client.read(4096);
    fmt::printf("Output: %s\n", std::string(output.begin(), output.end()));

    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    neco::run(argc, argv, main_);
}
