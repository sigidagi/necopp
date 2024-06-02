#include <fmt/core.h>
#include "necopp.hpp"
#include <unistd.h>

int main_(int, char **) {
    fmt::print("Hello, coroutine!\n");

    int fd = neco::dial("tcp", "scanme.nmap.org:80");
    if (fd < 0) {
        fmt::print("Failed to dial: {}\n", fd);
        return 1;
    }
    
    fmt::print("Connected to scanme.nmap.org:80\n");

    close(fd);
    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
    return 0;
}
