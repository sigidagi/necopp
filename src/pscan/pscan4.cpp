#include <fmt/core.h>
#include <fmt/format.h>
#include "necopp.hpp"
#include <unistd.h>
#include <chrono>

using namespace std::chrono_literals;

int worker(int argc, void** argv) {
    (void)argc;
    auto wg = (neco::waitgroup*)argv[0];

    auto receiver = (neco_chan*)(argv[1]);
    int port;
    neco_chan_recv(receiver, &port);
    fmt::print("worker: {}\n", port);
    wg->done();

    return 0;
}

int main_(int argc, char** argv) {
    (void)argc;
    (void)argv;
    fmt::print("Hello, coroutine!\n");
    
    int cap = 100;
    auto sender = neco::channel<int>(cap);
    neco::waitgroup wg;

    for (int i = 0; i < cap; i++) {
        // Using C style argument passing
        neco::go(std::function<int(int, void**)>(&worker))(&wg, sender.get());
    }
    
    for (int i = 0; i < 1024; i++) {
        wg.add(1);
        sender.send(&i);
    }

    wg.wait();
    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
    return 0;
}
