#include <fmt/core.h>
#include <fmt/format.h>
#include "necopp.hpp"
#include "http.hpp"
#include "error.hpp"
#include <algorithm>

void receiver(int fd, const std::string& res_json) {

    try {
        neco::http io(fd);
        //neco::response res = io.receive();
        neco::response res;

        for (const auto& [key, value] : res.headers) {
            fmt::print("{}: {}\n", key, value);
        }
        fmt::print("Body: {}\n", res.body);

        // search for Content-Type header and determine the response Type
        // if not found, default to text/html
        auto it = std::find_if(res.headers.begin(), res.headers.end(), [](const auto& header) {
            return header.first == "Content-Type";
        });

        if (it == res.headers.end()) {
            fmt::print("Content-Type not found.\n");
        }


        size_t count = neco_write(fd, res_json.c_str(), res_json.size());
        if (count != res_json.size()) {
            throw neco::exception(neco::http_error::SEND_FAILED, "Failed to send response.");
        }

        io.send(res_json);
    }
    catch (const std::exception& e) {
        fmt::print("Error: {}\n", e.what());
    }
}

void receiver2(int fd, const std::string& res_json) {

    neco::io io(fd);
    std::vector<char> buf = io.read(4096);
    fmt::print("Response: {}\n", std::string(buf.begin(), buf.end()));

    io.write(res_json);
}

int main_(int, char **) {
    //
    int servfd = neco::serve("tcp", "127.0.0.1:5000");
    if (servfd < 0) {
        fmt::print("Failed to serve: {}\n", neco_strerror(errno));
        return 1;
    }

    fmt::print("Serving on: {}\n", "127.0.0.1:5000");
    while(true) {
        int fd = neco::accept(servfd, 0, 0);
        if (fd < 0) {
            fmt::print("Failed to accept: {}\n", neco_strerror(errno));
            return 1;
        }

        neco::go([&fd](int, void**) {

            std::string res_json =
                "HTTP/1.0 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: 27\r\n"
                "\r\n"
                "{\"greetings\":\"Hello Neco!\"}\n";

            receiver2(fd, res_json);
        })();
    }

    fmt::print("Closing neco server!\n");
    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
}
