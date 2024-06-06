#include "necopp.hpp"
#include <iostream>

int main_(int, char **) {
    //
    int servfd = neco::serve("tcp", "127.0.0.1:8080");
    if (servfd < 0) {
        std::cerr << "Failed to start server\n";
        return 1;
    }
    
    std::cout << "Serving on: 127.0.0.1:8080" << std::endl;
    while(true) {
        int fd = neco::accept(servfd, 0, 0);
        if (fd < 0) {
            std::cerr << "Failed to accept\n";
            return 1;
        }

        neco::go([&fd](int, void **) {
            neco::io io(fd);
            std::vector<char> buf = io.read(4096);
            std::cout << "Received: " << std::string(buf.begin(), buf.end()) << std::endl;
            
            std::string res_html = 
                "HTTP/1.0 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 21\r\n"
                "\r\n"
                "<h1>Hello Neco!</h1>\n";

            io.write(res_html);
        })();
    }

    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
}
