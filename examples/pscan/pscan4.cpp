#include <fmt/core.h>
#include <fmt/format.h>
#include "necopp.hpp"
#include <unistd.h>
#include <chrono>
#include "timer.hpp"

using namespace std::chrono_literals;

int worker(int, void** argv) {
    auto wg = (neco::waitgroup*)argv[0];
    auto ch = static_cast<neco::channel<int>*>(argv[1]);
    
    int port{};
    while (true) {
        ch->receiver >> port;
        fmt::print("port {}\n", port);
        wg->done();
    }

    return 0;
}

int main_(int, char **) {
    //
    int cap = 100;
    auto ch = neco::channel<int>(cap);
    neco::waitgroup wg;
    
    // Spin 100 workers
    for (int i = 0; i < cap; i++) {
        // Using C style argument passing
        neco::go(std::function<int(int, void**)>(&worker))(&wg, &ch);
    }
    
    for (int i = 0; i < 1024; i++) {
        wg.add(1);
        ch.sender << i;
    }
    
    wg.wait();
    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
    return 0;
}
