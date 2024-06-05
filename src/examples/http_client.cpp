#include <unistd.h>
#include <fmt/printf.h>
#include "necopp.hpp"
#include <iostream>
#include "httpp.hpp"

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
    //fmt::printf("Output: %s\n", std::string(output.begin(), output.end()));

    httpp::parser parser;
    parser.execute({output.data(), output.size()});
    
    auto res = parser.response();

    for (const auto& [key, value] : res.headers) {
        fmt::print("{}: {}\n", key, value);
    }

    fmt::print("\n{}\n", res.body);

    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    neco::run(argc, argv, main_);
}
