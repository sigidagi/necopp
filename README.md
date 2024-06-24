<p align="center">
<picture>
  <img alt="Neco" src="assets/neco++3new.png" width="240">
</picture>
</p>
<p></p>

This library is a C++ wrapper around the [Neco](https://github.com/tidwall/neco) library.
Concurrent programming in C++ is not easy, and with a help of a library like Neco, it can be made easier.
The goal is to provide similar interface to the one provided by the Go language, but in C++.


## Using

```sh
git clone https://github.com/sigidagi/necopp.git
cd necopp
cmake -B build -S .
cmake --build build
```


## Example 1: Start a coroutine

```c++
#include "necopp.hpp"
#include <iostream>

int main_(int, char **) {
    std::cout << "Hello, coroutine!\n";
    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    return (int)neco::run(argc, argv, main_);
}
```

## Example 2: Multiple coroutines

Two coroutines that continuously prints "tick" every one second and "tock" every two.

```c++
#include "necopp.hpp"
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

int main_(int, char **) {
   
    neco::go([](int, void **){
        while (true) {
            neco::sleep(1s);
            std::cout << "tick\n";
        }
    })();

    neco::go([](int, void **){
        while (true) {
            neco::sleep(2s);
            std::cout << "tock...\n";
        }
    })();

    // Keep the program alive for an hour.
    neco::sleep(1h);
    return 0;
}

int main(int argc, char* argv[]) {
    return (int)neco::run(argc, argv, main_);
}
```

## Example 3: Coroutine arguments

A coroutine is like its own little program that accepts any number of arguments.

```c++
#include "necopp.hpp"
#include <memory>

using namespace std::chrono_literals;

int main_(int, char **) {

    int arg0 = 0;
    auto arg1 = std::make_shared<int>(1);
    std::string arg2 = "hello world";

    neco::go([&arg0, &arg1, &arg2] (int, void** ) {
        // 
        std::cout << "arg0: " << arg0 << ", arg1: " << *arg1 << "', arg2: " << arg2 << "\n";
        neco::sleep(500ms);
        std::cout << "second done\n";
    })();
    
    // or passing arguments to coroutine function
    neco::go([] (int, void** argv) {
    
        int* arg0 = static_cast<int*>(argv[0]);
        int* arg1 = static_cast<int*>(argv[1]);
        const char* arg2 = static_cast<const char*>(argv[2]);

        std::cout << "arg0: " << *arg0 << ", arg1: " << *arg1 << "', arg2: " << arg2 << "\n";
        neco::sleep(500ms);
        std::cout << "third done\n";
    })(&arg0, arg1.get(), arg2.data());
   

    neco::sleep(1s);
    std::cout << "first done\n";
    return 0;
}

int main(int argc, char* argv[]) {
    return (int)neco::run(argc, argv, main_);
}
```

## Example 4: Channels

A `channel` is a mechanism for communicating between two or more coroutines.


```c++
#include "necopp.hpp"
#include <iostream>
#include <memory>
#include <chrono>

using namespace std::chrono_literals;

struct Foo {
    int one;
    double two;
    std::shared_ptr<int> ptr = nullptr;
    const char* say;
};

int main_(int, char **) {
    
    // create a channel which can receive Foo objects.
    auto chFoo = neco::channel<Foo>();
    neco::go([&](int, void **) {

        std::cout << "Hello from coroutine\n";
        // Some heavy work here
        // ...
        // return Foo object as a result
        chFoo.sender.send({ 42, 3.14, std::make_shared<int>(66), "Hello" });
    })();
    
    Foo foo = chFoo.receiver.recv();
    std::cout << "received: " << foo.say << ", " << *foo.ptr << "\n";
    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    neco::run(argc, argv, main_);
}

```

## Example 5: Generators

A generator is like channel but is bound to a coroutine and is intended to treat the coroutine like an iterator.

```c++
#include "necopp.hpp"
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

int main_(int, char **) {
    // create a generator of integers. 
    auto gen = neco::generator<int>([](int, void **) {
        // 
        for (int i = 0; i < 10; i++) {
            neco::sleep(300ms);
            neco::yield(i);
        }
    })();
    
    int i;
    while (gen.next(i) != neco::Result::CLOSED) {
        std::cout << "Got: " << i << std::endl;;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    return (int)neco::run(argc, argv, main_);
}
```

## Example 6: HTTP client 

```c++
#include "necopp.hpp"
#include <iostream>
#include <unistd.h>

using namespace std::chrono_literals;

int main_(int, char **) {

    int fd = neco::dial("tcp", "example.com:80");
    if (fd < 0) {
        std::cerr << "Failed to dial" << std::endl;
        return 1;
    }

    std::string req = 
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Connection: close\r\n"
        "\r\n";
    
    neco::io client{fd};
    // Send request
    client.write(req);
    // Read response
    auto output = client.read(4096);
    // Print response
    std::cout << std::string(output.data(), output.size()) << std::endl;
    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    neco::run(argc, argv, main_);
}
```

## Example 7: HTTP server 

```c++
#include "necopp.hpp"
#include <iostream>

int main_(int, char **) {
    //
    int servfd = neco::serve("tcp", "127.0.0.1:8080");
    if (servfd < 0) {
        std::cerr << "Failed to start server\n";
        return 1;
    }
    
    std::cout << "Serving on: 127.0.0.1:8080" << std::endl;
    while(true) {
        int fd = neco::accept(servfd, 0, 0);
        if (fd < 0) {
            std::cerr << "Failed to accept\n";
            return 1;
        }

        neco::go([&fd](int, void **) {
            neco::io io(fd);
            std::vector<char> buf = io.read(4096);
            std::cout << "Received: " << std::string(buf.begin(), buf.end()) << std::endl;
            
            std::string res_html = 
                "HTTP/1.0 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 21\r\n"
                "\r\n"
                "<h1>Hello Neco!</h1>\n";

            io.write(res_html);
        })();
    }

    return 0;
}

int main(int argc, char* argv[]) {
    neco::run(argc, argv, main_);
}
```

## Example 8: Suspending and resuming a coroutine

Any coroutines can suspended itself indefinetly and then be resumed by other
coroutines by using `neco::suspend` and `neco::resume`. 

```c++
#include "necopp.hpp"
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

int main_(int, char **) {
    
    neco::go([] (int, void**) {
        printf("Suspending coroutine\n");
        neco::suspend();
        printf("Coroutine resumed\n");
    })();

    for (int i = 0; i < 3; i++) {
        std::cout << i+1 << std::endl;
        neco::sleep(1s);
    }

    // Resume the suspended. The neco::lastid() returns the identifier for the
    // last coroutine started by the current coroutine.
    neco::resume(neco::lastid());
    return 0;
}

int main(int argc, char* argv[]) {
    return (int)neco::run(argc, argv, main_);
}
```

## Example 9: Select

The `select` statement lets a coroutine wait on multiple communication operations.

```c++
#include "necopp.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
#include <fmt/format.h>

using namespace std::chrono_literals;

int main_(int, char **) {
    
    neco::channel<const char*> ch1;
    neco::channel<const char*> ch2;

    neco::go([&](int, void**) {
        neco::sleep(500ms);
        ch2.sender.send("Hello");
    })();

    neco::go([&](int, void**) {
        neco::sleep(1s);
        ch1.sender.send("World");
    })();
    
    std::vector<std::string> words;
    
    neco::select(
        neco::incase{ch1, [&words](const auto &data) {
            words.push_back(data);
        }},
        neco::incase{ch2, [&words](const auto &data) {
            words.push_back(data);
        }}
    );
    
    fmt::print("Words: '");
    for (const auto &word : words) {
        fmt::print("{} ", word);
    }
    fmt::print("'\n");
    fflush(stdout);
    
    return 0;
}

int main(int argc, char* argv[]) {
    // run main program in Neco coroutine context
    neco::run(argc, argv, main_);
}
```

## License

Source code is available under the MIT [License](LICENSE).
