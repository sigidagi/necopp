#include <fmt/core.h>
#include <fmt/format.h>
#include "necopp.hpp"
#include <unistd.h>
#include <chrono>

using namespace std::chrono_literals;

int main_(int argc, char** argv) {
    (void)argc;
    (void)argv;
    fmt::print("Hello, coroutine!\n");

    for (int i = 70; i < 81; i++) {
        std::string host = fmt::format("scanme.nmap.org:{}", i);

        int fd = neco::dial("tcp", host.c_str(), 1s);
        if (fd < 0) {
            fmt::print("Host {} closed\n", host);
            continue;
        }
        fmt::print("Connected to {}\n", host);
        close(fd);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
    return 0;
}
