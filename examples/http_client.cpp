#include <unistd.h>
#include <fmt/printf.h>
#include "necopp.hpp"
#include "requests.hpp"

using namespace std::chrono_literals;

int main_(int, char **) {

// ----- High level API
    try {
        auto request = neco::requests("http://example.com:80");
        request.header("Connection", "close");

        auto response = request.get("/");

        fmt::print("HTTP/1.1 {} {}\n", response.status.code, response.status.message);
        for (const auto& [key, value] : response.headers) {
            fmt::print("{}: {}\n", key, value);
        }
        fmt::print("\n{}\n", response.body);
    }
    catch (const std::exception &e) {
        fmt::print("Exception: {}\n", e.what());
    }

/*
 *    try {
 *        auto request = neco::requests("http://localhost:5000");
 *        request.header("Accept", "application/json");
 *        request.header("Content-Type", "application/json");
 *        
 *        std::string data = R"({"key": "value"})";
 *        auto response = request.post("/echo", data);
 *
 *        fmt::print("HTTP/1.1 {} {}\n", response.status.code, response.status.message);
 *        for (const auto& [key, value] : response.headers) {
 *            fmt::print("{}: {}\n", key, value);
 *        }
 *        fmt::print("\n{}\n", response.body);
 *    }
 *    catch (const std::exception &e) {
 *        fmt::print("Exception: {}\n", e.what());
 *    }
 */


// ----- Low level API

/*
 *
 *    int fd = neco::dial("tcp", "example.com:80");
 *    if (fd < 0) {
 *        fmt::print("neco_dial: %s\n", neco_strerror(fd));
 *        return 0;
 *    }
 *
 *
 *    std::string req = 
 *        "GET / HTTP/1.1\r\n"
 *        "Host: example.com\r\n"
 *        "Connection: close\r\n"
 *        "\r\n";
 *    
 *    neco::io client{fd};
 *    
 *    client.write(req);
 *    auto output = client.read(4096);
 *
 *
 *    close(fd);
 */
    return 0;
}

int main(int argc, char *argv[]) {
    return (int)neco::run(argc, argv, main_);
}
