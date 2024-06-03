#include "hparser.hpp"
#include <fmt/format.h>

int main() {
    hparser parser;
    parser.on_message([](llhttp_t *) {
        fmt::print("Message complete\n");
        return 0;
    });

    parser.execute("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
    return 0;
}
