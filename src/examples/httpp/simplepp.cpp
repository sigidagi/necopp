#include "httpp.hpp"
#include <fmt/format.h>

int main() {
    httpp::parser parser;
    parser.execute("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");

    auto res = parser.response();
    fmt::print("Status: {}\n", res.status_code);

    return 0;
}
