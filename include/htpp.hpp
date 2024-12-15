#pragma once

#include <arpa/inet.h>
#include <llhttp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>

namespace nl = nlohmann;

namespace htpp {

class HttpParser {
   public:
    using Callback = std::function<int()>;
    using DataCallback = std::function<int(const char*, size_t)>;

    enum class Type {
        REQUEST = HTTP_REQUEST,
        RESPONSE = HTTP_RESPONSE,
        BOTH = HTTP_BOTH
    };

    enum class Error {
        OK = HPE_OK,
        INTERNAL = HPE_INTERNAL,
        STRICT = HPE_STRICT,
        PAUSED = HPE_PAUSED
    };

    explicit HttpParser(Type type) {
        llhttp_settings_init(&settings_);
        llhttp_init(&parser_, static_cast<llhttp_type_t>(type), &settings_);
        parser_.data = this;

        settings_.on_message_begin = [](llhttp_t* parser) -> int {
            return static_cast<HttpParser*>(parser->data)->onMessageBegin_
                       ? static_cast<HttpParser*>(parser->data)
                             ->onMessageBegin_()
                       : 0;
        };
        settings_.on_message_complete = [](llhttp_t* parser) -> int {
            return static_cast<HttpParser*>(parser->data)->onMessageComplete_
                       ? static_cast<HttpParser*>(parser->data)
                             ->onMessageComplete_()
                       : 0;
        };
        settings_.on_header_field = [](llhttp_t* parser, const char* at,
                                       size_t length) -> int {
            return static_cast<HttpParser*>(parser->data)->onHeaderField_
                       ? static_cast<HttpParser*>(parser->data)
                             ->onHeaderField_(at, length)
                       : 0;
        };
        settings_.on_url = [](llhttp_t* parser, const char* at,
                              size_t length) -> int {
            return static_cast<HttpParser*>(parser->data)->onUrl_
                       ? static_cast<HttpParser*>(parser->data)
                             ->onUrl_(at, length)
                       : 0;
        };
    }

    ~HttpParser() = default;

    void onMessageBegin(Callback callback) {
        onMessageBegin_ = std::move(callback);
    }

    void onMessageComplete(Callback callback) {
        onMessageComplete_ = std::move(callback);
    }

    void onHeaderField(DataCallback callback) {
        onUrl_ = std::move(callback);
        settings_.on_header_field = [](llhttp_t* parser, const char* at, size_t length) -> int
        {
            auto* self = static_cast<HttpParser*>(parser->data);
            if (self->onHeaderField_) {
                return self->onHeaderField_(at, length);
            }
            return 0;
        };
    }

    void onHeaderValue(DataCallback callback) {
        onUrl_ = std::move(callback);
        settings_.on_header_value = [](llhttp_t* parser, const char* at, size_t length) -> int
        {
            auto* self = static_cast<HttpParser*>(parser->data);
            if (self->onHeaderValue_) {
                return self->onHeaderValue_(at, length);
            }
            return 0;
        };
    }

    void onUrl(DataCallback callback) {
        onUrl_ = std::move(callback);
        settings_.on_url = [](llhttp_t* parser, const char* at, size_t length) -> int
        {
            auto* self = static_cast<HttpParser*>(parser->data);
            if (self->onUrl_) {
                return self->onUrl_(at, length);
            }
            return 0;
        };
    }

    void onBody(DataCallback callback) {
        onBody_ = std::move(callback);
        settings_.on_body = [](llhttp_t* parser, const char* at,
                               size_t length) -> int {
            auto* self = static_cast<HttpParser*>(parser->data);
            if (self->onBody_) {
                return self->onBody_(at, length);
            }
            return 0;
        };
    }

    Error execute(std::string_view data) {
        llhttp_errno_t err = llhttp_execute(&parser_, data.data(), data.size());
        return static_cast<Error>(err);
    }

    Error finish() {
        llhttp_errno_t err = llhttp_finish(&parser_);
        return static_cast<Error>(err);
    }

    const char* errorReason() const {
        return llhttp_errno_name(static_cast<llhttp_errno_t>(parser_.error));
    }

   private:
    llhttp_t parser_;
    llhttp_settings_t settings_;

    Callback onMessageBegin_;
    Callback onMessageComplete_;

    DataCallback onHeaderField_;
    DataCallback onHeaderValue_;
    DataCallback onUrl_;
    DataCallback onBody_;
};

struct HttpResponse {
    int status = 0;  // HTTP status code (e.g., 200, 404)
    std::vector<std::pair<std::string, std::string>> headers;
    std::string raw;  // Raw response body
    nl::json body;
    std::string error;  // Error message (empty if no error)
};

/*
 *inline HttpParser createHttpParser(HttpResponse& response) {
 *    HttpParser parser(HttpParser::Type::RESPONSE);
 *    std::string currentHeaderField;
 *    std::string currentHeaderValue;
 *
 *    parser.onHeaderField([&](const char* at, size_t length) {
 *        std::cout << "Header Field Pointer: " << static_cast<const void*>(at) << ", Length: " << length << "\n";
 *        currentHeaderField.assign(at, length); // Log the operation
 *        std::cout << "Current Header Field: " << currentHeaderField << "\n";
 *        return 0;
 *    });
 *
 *    parser.onHeaderValue([&](const char* at, size_t length) {
 *        std::cout << "Header Value Pointer: " << static_cast<const void*>(at) << ", Length: " << length << "\n";
 *        currentHeaderValue.assign(at, length); // Log the operation
 *        response.headers.emplace_back(currentHeaderField, currentHeaderValue); // Log the header pair
 *        std::cout << "Added Header: " << currentHeaderField << ": " << currentHeaderValue << "\n";
 *        currentHeaderField.clear();
 *        return 0;
 *    });
 *
 *    parser.onBody([&](const char* at, size_t length) {
 *        if (at == nullptr || length == 0) {
 *            std::cerr << "Error: Invalid body data (null pointer or zero length).\n";
 *            return -1; // Signal error to llhttp
 *        }
 *        if (std::strlen(at) < length) { // Ensure `length` is within bounds
 *            std::cerr << "Error: Body length exceeds available data.\n";
 *            return -1;
 *        }
 *
 *        try {
 *            response.raw.append(at, length);
 *        } catch (const std::exception& e) {
 *            std::cerr << "Error appending body data: " << e.what() << "\n";
 *            return -1;
 *        }
 *        return 0;
 *    });
 *
 *    parser.onMessageComplete([&]() {
 *        if (!currentHeaderField.empty()) {
 *            response.headers.emplace_back(currentHeaderField,
 *                                        currentHeaderValue);
 *        }
 *        
 *        if (nlohmann::json::accept(response.raw)) {
 *            try {
 *                response.body = nlohmann::json::parse(response.raw); // Parse JSON
 *            } catch (const std::exception& e) {
 *                response.error = "Failed to parse JSON body: " + std::string(e.what());
 *            }
 *        }
 *        return 0;
 *    });
 *
 *    return parser;
 *}
 */

inline HttpResponse request(const std::string& host, int port, const std::string& request) {
    HttpResponse response;
    // Prepare llhttp parser
    //htpp::HttpParser parser = createHttpParser(response);

    try {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            response.error = "Socket creation failed.";
            return response;
        }

        struct sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);

        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock);
            response.error = "Connection failed.";
            return response;
        }

        if (::send(sock, request.c_str(), request.size(), 0) < 0) {
            close(sock);
            response.error = "Failed to send request.";
            return response;
        }

        HttpParser parser(HttpParser::Type::RESPONSE);
        std::string currentHeaderField;
        std::string currentHeaderValue;

        parser.onHeaderField([&](const char* at, size_t length) {
            std::cout << "Header Field Pointer: " << static_cast<const void*>(at) << ", Length: " << length << "\n";
            currentHeaderField.assign(at, length); // Log the operation
            std::cout << "Current Header Field: " << currentHeaderField << "\n";
            return 0;
        });

        parser.onHeaderValue([&](const char* at, size_t length) {
            std::cout << "Header Value Pointer: " << static_cast<const void*>(at) << ", Length: " << length << "\n";
            currentHeaderValue.assign(at, length); // Log the operation
            response.headers.emplace_back(currentHeaderField, currentHeaderValue); // Log the header pair
            std::cout << "Added Header: " << currentHeaderField << ": " << currentHeaderValue << "\n";
            currentHeaderField.clear();
            return 0;
        });

        parser.onBody([&](const char* at, size_t length) {
            if (at == nullptr || length == 0) {
                std::cerr << "Error: Invalid body data (null pointer or zero length).\n";
                return -1; // Signal error to llhttp
            }
            if (std::strlen(at) < length) { // Ensure `length` is within bounds
                std::cerr << "Error: Body length exceeds available data.\n";
                return -1;
            }

            try {
                response.raw.append(at, length);
            } catch (const std::exception& e) {
                std::cerr << "Error appending body data: " << e.what() << "\n";
                return -1;
            }
            return 0;
        });

        parser.onMessageComplete([&]() {
            if (!currentHeaderField.empty()) {
                response.headers.emplace_back(currentHeaderField,
                                            currentHeaderValue);
            }
            
            if (nlohmann::json::accept(response.raw)) {
                try {
                    response.body = nlohmann::json::parse(response.raw); // Parse JSON
                } catch (const std::exception& e) {
                    response.error = "Failed to parse JSON body: " + std::string(e.what());
                }
            }
            return 0;
        });

        // Read response in chunks
        char buffer[2048];
        ssize_t bytes_received;
        while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
            HttpParser::Error err = parser.execute(std::string_view(buffer, bytes_received));
            if (err != HttpParser::Error::OK) {
                response.error = "HTTP parsing error: " + std::string(parser.errorReason());
                close(sock);
                return response;
            }
        }
        
        if (bytes_received < 0) {
            response.error = "Socket read error.";
        }
        
        close(sock);
    } catch (const std::exception& e) {
        response.error = e.what();
    }

    return response;
}

class RequestBuilder {
   public:
    RequestBuilder(const std::string& url) { parseUrl(url); }

    RequestBuilder& json(const nlohmann::json& body) {
        body_ = body.dump();
        return *this;
    }

    HttpResponse sendRequest() {
        std::ostringstream request;

        // Start line
        request << (body_.empty() ? "GET " : "POST ") << path_
                << " HTTP/1.1\r\n";

        // Headers
        request << "Host: " << host_ << "\r\n";
        request << "Content-Type: application/json\r\n";
        if (!body_.empty()) {
            request << "Content-Length: " << body_.size() << "\r\n";
        }

        request << "Connection: close\r\n\r\n";

        // Body
        if (!body_.empty()) {
            request << body_;
        }

        return htpp::request(host_, port_, request.str());
    }

   private:
    void parseUrl(const std::string& url) {
        static const std::regex url_regex(
            R"(^http://([^:/]+)(?::(\d+))?(/.*)?$)");
        std::smatch match;
        if (!std::regex_match(url, match, url_regex)) {
            throw std::invalid_argument("Invalid URL format");
        }
        host_ = match[1].str();
        port_ = match[2].matched ? std::stoi(match[2].str()) : 80;
        path_ = match[3].matched ? match[3].str() : "/";
    }

    std::string host_;
    int port_ = 80;
    std::string path_;
    std::string body_;
};

inline HttpResponse get(const std::string& url) {
    RequestBuilder builder(url);
    return builder.sendRequest();
}

inline HttpResponse post(const std::string& url, const nlohmann::json& body) {
    RequestBuilder builder(url);
    builder.json(body);
    return builder.sendRequest();
}

}  // namespace llhttp

