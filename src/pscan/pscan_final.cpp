#include <fmt/core.h>
#include <fmt/format.h>
#include "necopp.hpp"
#include <unistd.h>
#include <chrono>

using namespace std::chrono_literals;
const int WORKER_COUNT = 300;
const int PORT_COUNT = 1024;

int main_(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // pors and results channels
    auto ports = neco::channel<int>::create(WORKER_COUNT);
    auto results = neco::channel<int>::create(WORKER_COUNT);
    

    // 1. Spin workers
    for (int i = 0; i < WORKER_COUNT; i++) {
        // Using C style argument passing
        neco::go([&](int argc, void** argv) {
            (void)argc;
            (void)argv;
            
            const auto& rsender = results.sender;
            const auto& preceiver = ports.receiver;
            
            int none = 0, port = 0;
            while (true) {
                preceiver >> port;
                std::string host = fmt::format("scanme.nmap.org:{}", port);

                int fd = neco::dial("tcp", host.c_str(), 1s);
                if (fd < 0) {
                    rsender << none;
                }
                else {
                    rsender << port;
                    close(fd);
                }
            }
       })();

    } // for loop
    
    // 2. Send ports to workers
    const auto& port_sender = ports.sender;
    neco::go([&](int argc, void** argv) {
        (void)argc;
        (void)argv;
        
        for (int i = 1; i < PORT_COUNT; i++) {
            port_sender << i;
        }
    })();
    
    
    // 3. Listen on channel to receive results.

    const auto& result_receiver = results.receiver;

    std::vector<int> open_ports;
    int port;
    // It was send 1024 times, so we need to receive 1024 times
    for (int i = 1; i < PORT_COUNT; i++) {
        result_receiver >> port;
        if (port != 0) {
            open_ports.push_back(port);
        }
    } 
    
    fmt::print("Finished:\n");
    std::sort(open_ports.begin(), open_ports.end());
    for (auto p : open_ports) {
        fmt::print("Port '{}' is open\n", p);
    }
    
    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
    return 0;
}
