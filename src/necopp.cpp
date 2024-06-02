#include "necopp.hpp"
#include <iostream>
#include <unistd.h> // close(fd)

std::function<void(int, void**)>* globalFunction = nullptr;

namespace neco 
{
    // Nano seconds duration 
    Result sleep(duration duration) { return (Result)neco_sleep(duration.count()); }
    Result sleep(duration duration, std::function<Result()> func) {
        neco::sleep(duration);
        return func();
    }

    Result suspend() { return (Result)neco_suspend(); }
    Result suspend(duration deadline) {return (Result)neco_suspend_dl(deadline.count()); }
    Result resume(int64_t id) { return (Result)neco_resume(id); }
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
    

#include <iostream>

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

/*
 *    Result select_impl(std::initializer_list<neco::channel*>&& chans) {
 *        return (Result)neco_chan_select(chans.size(), chans.begin());
 *    }
 *
 */
    // --------------------------------------------------------------------------------------------
    go::go(coroutine coro) : m_callback(coro) {}

    void go::wrapper(int argc, void** argv) {
        if (globalFunction == nullptr) {
            return;
        }
        (*globalFunction)(argc, argv);
    }
    
    Result run(int argc, char* argv[], int (*user_main)(int, char**)) {
        neco_env_setpaniconerror(true);
        neco_env_setcanceltype( NECO_CANCEL_ASYNC ); 
        
        auto coro = neco::go([user_main](int argc, void** argv) {
            (void)argc;
            __neco_exit_prog(user_main(*(int*)argv[0], *(char***)argv[1]));
        });
        
        return coro(&argc, &argv);
    }

    waitgroup::waitgroup() {
        neco_waitgroup_init(&m_waitgroup);
    }
    
    Result waitgroup::add(int delta) {
        return (Result)neco_waitgroup_add(&m_waitgroup, delta);
    }

    Result waitgroup::done() {
        return (Result)neco_waitgroup_done(&m_waitgroup);
    }

    Result waitgroup::wait() {
        return (Result)neco_waitgroup_wait(&m_waitgroup);
    }

    Result waitgroup::wait(duration duration) {
        return (Result)neco_waitgroup_wait_dl(&m_waitgroup, duration.count());
    }

} // namespace neco

    /*
     *void adapter(int argc, void** argv) {
     *    auto func = *reinterpret_cast<std::function<void(int, void**)>*>(argv[0]);
     *    void** args = reinterpret_cast<void**>(argv[1]);
     *    func(argc -1, args);
     *}
     */

