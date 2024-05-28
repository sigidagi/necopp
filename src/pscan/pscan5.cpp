#include <fmt/core.h>
#include <fmt/format.h>
#include "necopp.hpp"
#include <unistd.h>
#include <chrono>

using namespace std::chrono_literals;

int worker(int argc, void** argv) {
    (void)argc;
    auto port_receiver = neco::channel<int>{static_cast<neco_chan*>(argv[0])};
    auto result_sender = neco::channel<int>{static_cast<neco_chan*>(argv[1])};
    // send 0 if port is closed 
    int none = 0;

    while (true) {
        int port = *port_receiver.recv();
        std::string host = fmt::format("scanme.nmap.org:{}", port);

        int fd = neco::dial("tcp", host.c_str(), 1s);
        if (fd < 0) {
            result_sender.send(&none);
        }
        else {
            fmt::print("Connected to {}\n", host);
            result_sender.send(&port);
            close(fd);
        }
    }

    return 0;
}

int main_(int argc, char** argv) {
    (void)argc;
    (void)argv;
    fmt::print("Hello, coroutine!\n");
    
    int cap = 100;
    auto port_sender = neco::channel<int>(cap);
    auto result_receiver = neco::channel<int>(cap);
    
    // Spin 100 workers
    for (int i = 0; i < cap; i++) {
        // Using C style argument passing
        neco::go(
            std::function<int(int, void**)>(&worker))(port_sender.get(), result_receiver.get()
        );
    }
    
    neco::go([&port_sender](int argc, void** argv) {
        (void)argc;
        (void)argv;
        
        for (int i = 0; i < 1024; i++) {
            port_sender.send(&i);
        }
    })();
    
    std::vector<int> ports;

    // It was send 1024 times, so we need to receive 1024 times
    for (int i = 0; i < 1024; i++) {
        int port = *result_receiver.recv();
        if (port != 0) {
            ports.push_back(port);
        }
    } 

    std::sort(ports.begin(), ports.end());
    for (auto port : ports) {
        fmt::print("Port '{}' is open\n", port);
    }
    
    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
    return 0;
}
