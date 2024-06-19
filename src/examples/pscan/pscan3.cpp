#include <fmt/core.h>
#include <fmt/format.h>
#include "necopp.hpp"
#include <unistd.h>
#include <chrono>
#include "timer.hpp"

using namespace std::chrono_literals;

int main_(int, char **) {
    // synchronized counter - waitgroup
    auto timer = Timer();
    neco::waitgroup wg;
    for (int i = 1; i < 100; i++) {
        //        
        wg.add(1);
        neco::go([&i, &wg] (int, void**) {
            // 
            std::string host = fmt::format("scanme.nmap.org:{}", i);

            int fd = neco::dial("tcp", host.c_str(), 1s);
            if (fd < 0) {
                fmt::print("."); std::fflush(nullptr);
            }
            else {
                fmt::print("\nConnected to {}\n", host);
                std::fflush(nullptr);
                close(fd);
            }
            wg.done();
        })();
    } // for loop
    
    wg.wait();

    fmt::print("\nElapsed time: {} [ms]\n", timer.elapsed());
    std::fflush(nullptr);
    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
    return 0;
}
