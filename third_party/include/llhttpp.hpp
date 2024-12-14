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

namespace llhttp {

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

    void onHeaderField(std::function<int(const char*, size_t)> callback) {
        onHeaderField_ = std::move(callback);
        settings_.on_header_field = [](llhttp_t* parser, const char* at,
                                       size_t length) -> int {
            auto* self = static_cast<HttpParser*>(parser->data);
            if (self->onHeaderField_) {
                return self->onHeaderField_(at, length);
            }
            return 0;
        };
    }

    void onHeaderValue(std::function<int(const char*, size_t)> callback) {
        onHeaderValue_ = std::move(callback);
        settings_.on_header_value = [](llhttp_t* parser, const char* at,
                                       size_t length) -> int {
            auto* self = static_cast<HttpParser*>(parser->data);
            if (self->onHeaderValue_) {
                return self->onHeaderValue_(at, length);
            }
            return 0;
        };
    }

    void onUrl(std::function<int(const char*, size_t)> callback) {
        onUrl_ = std::move(callback);
        settings_.on_url = [](llhttp_t* parser, const char* at,
                              size_t length) -> int {
            auto* self = static_cast<HttpParser*>(parser->data);
            if (self->onUrl_) {
                return self->onUrl_(at, length);
            }
            return 0;
        };
    }

    void onBody(std::function<int(const char*, size_t)> callback) {
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

    std::function<int(const char*, size_t)> onHeaderField_;
    std::function<int(const char*, size_t)> onHeaderValue_;
    std::function<int(const char*, size_t)> onUrl_;
    std::function<int(const char*, size_t)> onBody_;
};

struct HttpResponse {
    int status = 0;  // HTTP status code (e.g., 200, 404)
    std::vector<std::pair<std::string, std::string>> headers;
    nlohmann::json body;
    std::string error;  // Error message (empty if no error)
};

inline HttpResponse parseHttpResponse(const std::string& response) {
    HttpResponse result;
    std::string currentHeaderField;
    std::string currentHeaderValue;
    std::string body;

    try {
        llhttp::HttpParser parser(llhttp::HttpParser::Type::RESPONSE);

        parser.onHeaderField([&](const char* at, size_t length) {
            if (!currentHeaderField.empty()) {
                result.headers.emplace_back(currentHeaderField,
                                            currentHeaderValue);
                currentHeaderField.clear();
                currentHeaderValue.clear();
            }
            currentHeaderField.append(at, length);
            return 0;
        });

        parser.onHeaderValue([&](const char* at, size_t length) {
            currentHeaderValue.append(at, length);
            return 0;
        });

        parser.onBody([&](const char* at, size_t length) {
            body.append(at, length);
            return 0;
        });

        parser.onMessageComplete([&]() {
            if (!currentHeaderField.empty()) {
                result.headers.emplace_back(currentHeaderField,
                                            currentHeaderValue);
            }
            try {
                result.body = nlohmann::json::parse(body);
            } catch (const std::exception& e) {
                result.error =
                    "Failed to parse JSON body: " + std::string(e.what());
            }
            return 0;
        });

        auto error = parser.execute(response);
        if (error != llhttp::HttpParser::Error::OK) {
            result.error =
                "HTTP parsing error: " + std::string(parser.errorReason());
        }

        // Extract status code from the response
        size_t statusPos = response.find(' ') + 1;
        if (statusPos != std::string::npos) {
            size_t statusEnd = response.find(' ', statusPos);
            if (statusEnd != std::string::npos) {
                result.status = std::stoi(
                    response.substr(statusPos, statusEnd - statusPos));
            }
        }
    } catch (const std::exception& e) {
        result.error = e.what();
    }

    return result;
}

inline HttpResponse sendRequest(const std::string& host, int port,
                                const std::string& request) {
    HttpResponse response;

    try {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            response.error = "Socket creation failed.";
            return response;
        }

        struct sockaddr_in server_addr {};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);

        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) <
            0) {
            close(sock);
            response.error = "Connection failed.";
            return response;
        }

        if (::send(sock, request.c_str(), request.size(), 0) < 0) {
            close(sock);
            throw std::runtime_error("Failed to send request.");
        }

        // send(sock, request.c_str(), request.size(), 0);

        char buffer[4096];
        std::ostringstream responseStream;
        int bytes_received;
        while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
            responseStream.write(buffer, bytes_received);
        }

        close(sock);
        response = parseHttpResponse(responseStream.str());
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

        return llhttp::sendRequest(host_, port_, request.str());
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

inline std::string send(const std::string& host, int port,
                        const std::string& request) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw std::runtime_error("Socket creation failed.");
    }

    struct sockaddr_in server_addr {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) <
        0) {
        close(sock);
        throw std::runtime_error("Connection failed.");
    }

    if (::send(sock, request.c_str(), request.size(), 0) < 0) {
        close(sock);
        throw std::runtime_error("Failed to send request.");
    }

    char buffer[4096];
    std::ostringstream response;
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        response.write(buffer, bytes_received);
    }

    close(sock);
    return response.str();
}

}  // namespace llhttp

