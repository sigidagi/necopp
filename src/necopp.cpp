#include "necopp.hpp"
#include <iostream>
#include <unistd.h> // close(fd)

std::function<void(int, void**)>* globalNecoFunction = nullptr;

namespace neco 
{
    // Nano seconds duration 
    result sleep(duration duration) { return (result)neco_sleep(duration.count()); }
    result sleep(duration duration, std::function<result()> func) {
        neco::sleep(duration);
        return func();
    }

    result suspend() { return (result)neco_suspend(); }
    result suspend(duration deadline) {return (result)neco_suspend_dl(deadline.count()); }
    result resume(int64_t id) { return (result)neco_resume(id); }
    int64_t getid() { return neco_getid(); }
    int64_t lastid() { return neco_lastid(); }
    int64_t starterid() { return neco_starterid();}
    void exit() { neco_exit();}

    int serve(std::string_view network, std::string_view address) {
        return neco_serve(network.data(), address.data());
    }

    int serve(std::string_view network, std::string_view address, duration deadline) {
        return neco_serve_dl(network.data(), address.data(), deadline.count());
    }
    
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
        return neco_accept(sockfd, addr, addrlen);
    }

    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen, duration deadline) {
        return neco_accept_dl(sockfd, addr, addrlen, deadline.count());
    }

    int dial(std::string_view network, std::string_view address) {
        return neco_dial(network.data(), address.data());
    }
    
    int dial(std::string_view network, std::string_view address, duration deadline) {
        seconds sec = std::chrono::duration_cast<seconds>(deadline);
        int64_t dl = neco_now() + sec.count()*1000000000;
        return neco_dial_dl(network.data(), address.data(), dl);
    }
    
    io::io(int fd) : m_fd(fd) {
    }
    
    io::~io() {
        if (m_fd != -1) {
            ::close(m_fd);
        }
    }
    
    std::vector<char> io::read(size_t size) {
        std::vector<char> buf(size);
        int n = neco_read(m_fd, buf.data(), buf.size());
        if (n <= 0) {
            return {};
        }
        return  buf;
    }

    std::vector<char> io::read(size_t size, duration deadline) {
        std::vector<char> buf(size);
        ssize_t bytes = neco_read_dl(m_fd, buf.data(), buf.size(), deadline.count());
        if (bytes == -1) {
            return {};
        }
        return buf;
    }

    ssize_t io::write(const std::vector<char>& buf) {
        return neco_write(m_fd, buf.data(), buf.size());
    }
    ssize_t io::write(std::string_view buf) {
        return neco_write(m_fd, buf.data(), buf.size());
    }
    ssize_t io::write(const std::vector<char>& buf, duration deadline) {
        return neco_write_dl(m_fd, buf.data(), buf.size(), deadline.count());
    }

    // --------------------------------------------------------------------------------------------
    go::go(coroutine coro) {
       m_callback = convertToFunctionPointer(coro);
    }

    void go::wrapper(int argc, void** argv) {
        if (globalNecoFunction == nullptr) {
            return;
        }
        (*globalNecoFunction)(argc, argv);
    }
    
    result run(int argc, char* argv[], int (*user_main)(int, char**)) {
        neco_env_setpaniconerror(true);
        neco_env_setcanceltype( NECO_CANCEL_ASYNC ); 
        
        auto coro = neco::go([user_main](int argc, void** argv) {
            (void)argc;
            __neco_exit_prog(user_main(*static_cast<int*>(argv[0]), *static_cast<char***>(argv[1])));
        });
        
        return coro(&argc, &argv);
    }

    waitgroup::waitgroup() {
        neco_waitgroup_init(&m_waitgroup);
    }
    
    result waitgroup::add(int delta) {
        return (result)neco_waitgroup_add(&m_waitgroup, delta);
    }

    result waitgroup::done() {
        return (result)neco_waitgroup_done(&m_waitgroup);
    }

    result waitgroup::wait() {
        return (result)neco_waitgroup_wait(&m_waitgroup);
    }

    result waitgroup::wait(duration duration) {
        return (result)neco_waitgroup_wait_dl(&m_waitgroup, duration.count());
    }

} // namespace neco

    /*
     *void adapter(int argc, void** argv) {
     *    auto func = *reinterpret_cast<std::function<void(int, void**)>*>(argv[0]);
     *    void** args = reinterpret_cast<void**>(argv[1]);
     *    func(argc -1, args);
     *}
     */

