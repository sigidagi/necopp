#include <fmt/core.h>
#include <fmt/format.h>
#include "necopp.hpp"
#include <unistd.h>
#include <chrono>

using namespace std::chrono_literals;

int main_(int, char **) {
    // synchronized counter - waitgroup
    neco::waitgroup wg;
    
    for (int i = 1; i < 1024; i++) {
        //        
        wg.add(1);
            neco::go([] (int, void** argv){
                int i = *static_cast<int*>(argv[0]);
            auto wg = static_cast<neco::waitgroup*>(argv[1]);
            // 
            std::string host = fmt::format("scanme.nmap.org:{}", i);

            int fd = neco::dial("tcp", host.c_str(), 1s);
            if (fd < 0) {
                fmt::print("Port {} closed\n", i);
            }
            else {
                fmt::print("Connected to {}\n", host);
                close(fd);
            }
            wg->done();
        })(&i, &wg);
    } // for loop
    
    wg.wait();
    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
    return 0;
}
