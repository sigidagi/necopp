#include <string>
#include "htpp.hpp"
#include <fmt/format.h>
#include <nlohmann/json.hpp>

namespace nl = nlohmann;

int main() {
    try {
        // Define the URL and JSON body for the POST request
        std::string url = "http://localhost:5000/echo"; 
        nl::json jsonBody = {{"message", "Hello, Echo Server!"},
                             {"author", "llhttp C++ Wrapper"}};

        // Perform the POST request
        auto response = htpp::post(url, jsonBody);

        // Check if the request was successful
        if (!response.error.empty()) {
            fmt::print("Error: {}\n", response.error);
            return EXIT_FAILURE;
        }

        // Print the response details
        fmt::print("HTTP Status: {}\n", response.status);
        fmt::print("Headers size: {}\n", response.headers.size());
        for (const auto& header : response.headers) {
            fmt::print("{}: {}\n", header.first, header.second);
        }
        fmt::print("Body: {}\n", response.body.dump(4));

    } catch (const std::exception& e) {
        fmt::print("Exception occurred: {}", std::string(e.what()));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*
 *int main() {
 *    httpp::parser parser;
 *    parser.execute("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
 *
 *    auto res = parser.response();
 *    fmt::print("Status: {}\n", res.status_code);
 *
 *    return 0;
 *}
 */
