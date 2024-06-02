#include <fmt/core.h>
#include <fmt/format.h>
#include "necopp.hpp"
#include <unistd.h>
#include <chrono>

using namespace std::chrono_literals;
const int WORKER_COUNT = 300;
const int PORT_COUNT = 1024;

int worker(int, void** argv) {
    auto ports = static_cast<neco::channel<int>*>(argv[0]);
    auto results = static_cast<neco::channel<int>*>(argv[1]);
    // send 0 if port is closed 
    int none = 0;

    while (true) {
        int port = ports->receiver.recv();
        std::string host = fmt::format("scanme.nmap.org:{}", port);

        int fd = neco::dial("tcp", host.c_str(), 1s);
        if (fd < 0) {
            results->sender.send(none);
        }
        else {
            results->sender.send(port);
            close(fd);
        }
    }

    return 0;
}

int main_(int, char **) {
    // 
    auto ports = neco::channel<int>(WORKER_COUNT);
    auto results = neco::channel<int>(WORKER_COUNT);
    
    // Spin 100 workers
    for (int i = 0; i < WORKER_COUNT; i++) {
        // Using C style argument passing
        neco::go(std::function<int(int, void**)>(&worker))(&ports, &results);
    }
    
    neco::go([&ports](int, void **) {
        // 
        for (int i = 1; i < PORT_COUNT; i++) {
            ports.sender << i;
        }
    })();
    
    std::vector<int> open_ports;

    int port;
    // It was send 1024 times, so we need to receive 1024 times
    for (int i = 1; i < PORT_COUNT; i++) {
        results.receiver >> port;
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
