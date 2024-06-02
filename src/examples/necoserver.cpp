#include <fmt/core.h>
#include <fmt/format.h>
#include "necopp.hpp"


int main_(int, char **) {
    //
    int servfd = neco::serve("tcp", "127.0.0.1:8080");
    if (servfd < 0) {
        fmt::print("Failed to serve: {}\n", neco_strerror(errno));
        return 1;
    }
    
    fmt::print("Serving on: {}\n", "127.0.0.1:8080");
    while(true) {
        int fd = neco::accept(servfd, 0, 0);
        if (fd < 0) {
            fmt::print("Failed to accept: {}\n", neco_strerror(errno));
            return 1;
        }

        neco::go([&fd](int argc, void** argv) {
            (void)argc;
            (void)argv;

            neco::io io(fd);
            std::vector<char> buf = io.read(4096);
            fmt::print("Received: {}\n", std::string(buf.begin(), buf.end()));
            
            std::string res_html = 
                "HTTP/1.0 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 21\r\n"
                "\r\n"
                "<h1>Hello Neco!</h1>\n";

/*
 *            std::string res_json = 
 *                "HTTP/1.0 200 OK\r\n"
 *                "Content-Type: application/json\r\n"
 *                "Content-Length: 27\r\n"
 *                "\r\n"
 *                "{\"greetings\":\"Hello Neco!\"}\n";
 */
            io.write(res_html);
        })();
    }

    fmt::print("Hello, world!\n");
    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
}
