#include <fmt/core.h>
#include <fmt/format.h>
#include "necopp.hpp"
#include <unistd.h>
#include <chrono>
#include "timer.hpp"

const int PORT_NUM = 100;

using namespace std::chrono_literals;

int main_(int, char **) {

    auto timer = Timer();

    for (int i = 1; i < PORT_NUM; i++) {
        std::string host = fmt::format("scanme.nmap.org:{}", i);

        int fd = neco::dial("tcp", host.c_str(), 1s);
        if (fd < 0) {
            fmt::print("."); std::fflush(nullptr);
            continue;
        }
        fmt::print("\nConnected to '{}'\n", host);
        close(fd);

    }
    fmt::print("\nElapsed time: {} [ms]\n", timer.elapsed());
    std::fflush(nullptr);
    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
    return 0;
}
