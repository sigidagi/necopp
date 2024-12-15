#include <iostream>
#include <nlohmann/json.hpp>
#include "htpp.hpp"  // Adjust the include path if needed

int main() {
    try {
        // Define the URL and JSON body for the POST request
        std::string url = "http://localhost:5000/echo"; 
        nlohmann::json jsonBody = {{"message", "Hello, Echo Server!"},
                                   {"author", "llhttp C++ Wrapper"}};

        // Perform the POST request
        auto response = htpp::post(url, jsonBody);

        // Check if the request was successful
        if (!response.error.empty()) {
            std::cerr << "Error: " << response.error << "\n";
            return EXIT_FAILURE;
        }

        // Print the response details
        std::cout << "HTTP Status: " << response.status << "\n";
        std::cout << "Headers:\n";
        for (const auto& header : response.headers) {
            std::cout << header.first << ": " << header.second << "\n";
        }
        std::cout << "Body:\n" << response.body.dump(4) << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

