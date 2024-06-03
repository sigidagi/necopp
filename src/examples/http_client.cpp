#include <unistd.h>
#include <fmt/printf.h>
#include "necopp.hpp"
#include <iostream>
#include "hparser.hpp"

using namespace std::chrono_literals;

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

    hparser parser;
    parser.on_message([&parser](auto *) {
        fmt::print("Message complete\n");

        int status = parser.status_code();
        fmt::printf("Status code: %d\n", status);
    
        int method = parser.method();
        fmt::printf("Method: %d\n", method);
        
        int content_length = parser.content_length();
        fmt::printf("Content length: %d\n", content_length);
        fmt::print("\n\n");
        return 0;
    });
    
    parser.execute({output.data(), output.size()});

    fmt::printf("Output: %s\n", std::string(output.begin(), output.end()));
    
    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    neco::run(argc, argv, main_);
}
