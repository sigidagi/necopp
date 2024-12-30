#include "error.hpp"
#include "requests.hpp"
#include <llhttp.h>
#include <neco.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <regex>
#include <functional>
#include <string_view>
#include <string>
#include <iostream>

namespace neco {

class http_parser {
public:
    using callback = std::function<int()>;
    using data_callback = std::function<int(const char*, size_t)>;

    enum class error {
        OK = HPE_OK,
        INTERNAL = HPE_INTERNAL,
        STRICT = HPE_STRICT,
        PAUSED = HPE_PAUSED
    };

    http_parser() {
        llhttp_settings_init(&settings_);

        llhttp_init(&parser_, static_cast<llhttp_type_t>(HTTP_RESPONSE), &settings_);
        parser_.data = this;

        settings_.on_message_begin = [](llhttp_t* parser) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onMessageBegin_ ? self->onMessageBegin_() : 0;
        };

        settings_.on_message_complete = [](llhttp_t* parser) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onMessageComplete_ ? self->onMessageComplete_() : 0;
        };

        settings_.on_header_field = [](llhttp_t* parser, const char* at, size_t length) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onHeaderField_ ? self->onHeaderField_(at, length) : 0;
        };

        settings_.on_header_value = [](llhttp_t* parser, const char* at, size_t length) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onHeaderValue_ ? self->onHeaderValue_(at, length) : 0;
        };

        settings_.on_url = [](llhttp_t* parser, const char* at, size_t length) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onUrl_ ? self->onUrl_(at, length) : 0;
        };

        settings_.on_body = [](llhttp_t* parser, const char* at, size_t length) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onBody_ ? self->onBody_(at, length) : 0;
        };

        settings_.on_status = [](llhttp_t* parser, const char* at, size_t length) -> int {
            auto* self = static_cast<http_parser*>(parser->data);
            return self->onStatus_ ? self->onStatus_(at, length) : 0;
        };
    }
    ~http_parser() = default;

    void onMessageBegin(callback callback) {
        onMessageBegin_ = std::move(callback);
    }

    void onMessageComplete(callback callback) {
        onMessageComplete_ = std::move(callback);
    }

    void onHeaderField(data_callback callback) {
        onHeaderField_ = std::move(callback);
    }

    void onHeaderValue(data_callback callback) {
        onHeaderValue_ = std::move(callback);
    }

    void onUrl(data_callback callback) {
        onUrl_ = std::move(callback);
    }

    void onBody(data_callback callback) {
        onBody_ = std::move(callback);
    }

    void onStatus(data_callback callback) {
        onStatus_ = std::move(callback);
    }

    http_parser::error execute(std::string_view data) {
        llhttp_errno_t err = llhttp_execute(&parser_, data.data(), data.size());
        return static_cast<error>(err);
    }

    http_parser::error finish() {
        llhttp_errno_t err = llhttp_finish(&parser_);
        return static_cast<error>(err);
    }

    const char* errorReason() const {
        return llhttp_errno_name(static_cast<llhttp_errno_t>(parser_.error));
    }

    int statusCode() const {
        return parser_.status_code;
    }

private:
    llhttp_t parser_;
    llhttp_settings_t settings_;

    callback onMessageBegin_;
    callback onMessageComplete_;

    data_callback onHeaderField_;
    data_callback onHeaderValue_;
    data_callback onUrl_;
    data_callback onBody_;
    data_callback onStatus_;
};

requests::requests(const std::string& url) {
    
    static const std::regex url_regex(R"(^http://([^:/]+)(?::(\d+))?(/.*)?$)");
    std::smatch match;
    if (!std::regex_match(url, match, url_regex)) {
        throw exception(http_error::INVALID_URL, "Invalid URL format");
    }
    host_ = match[1].str();
    std::string endpoint = host_ + ":" +  (match[2].matched ? match[2].str() : "80");

    sock_ = neco_dial("tcp", endpoint.c_str());
    if (sock_ < 0) {
        throw exception(http_error::SOCKET_INIT_FAILED, "Socket creation failed.");
    }

    headers_ = {
        {"User-Agent", "neco-request/1.0"},
        {"Accept", "*/*"},
        {"Connection", "keep-alive"}
    };
}

requests::~requests() {
    if (sock_ >= 0) {
        close(sock_);
    }
}

void requests::parse(http_parser* parser) {
    // Read response in chunks
    char buffer[1024];
    ssize_t bytes_received;

    while ((bytes_received = neco_read(sock_, buffer, sizeof(buffer))) > 0) {
        http_parser::error err = parser->execute(std::string_view(buffer, bytes_received));
        if (err != http_parser::error::OK) {
            throw exception(http_error::HTTP_PARSING_FAILED,
                    "HTTP parsing error: " + std::string(parser->errorReason()));
        }
    }

    if (bytes_received < 0) {
        throw exception(http_error::SOCKET_READ_FAILED,
                "Socket read error: " + std::string(strerror(errno)));
    }

    http_parser::error err = parser->finish();
    if (err != http_parser::error::OK) {
        throw exception(http_error::HTTP_PARSING_FAILED,
                "HTTP parsing error: " + std::string(parser->errorReason()));
    }
}

response requests::send(const std::string& data) {
    
    size_t count = neco_write(sock_, data.c_str(), data.size());
    if (count < 0 || count != data.size()) {
        throw exception(http_error::SEND_FAILED, "Failed to send request.");
    }

    response response;
    http_parser parser;
    std::string currentHeaderField;
    std::string currentHeaderValue;

    parser.onHeaderField([&](const char* at, size_t length) {
        if (at == nullptr || length == 0) {
            throw exception(http_error::INVALID_HEADER_FIELD,
                    "Invalid header field (null pointer or zero length).");
        }
        currentHeaderField.assign(at, length); // Log the operation
        return 0;
    });

    parser.onHeaderValue([&](const char* at, size_t length) {
        if (at == nullptr || length == 0) {
            throw exception(http_error::INVALID_HEADER_VALUE,
                    "Invalid header value (null pointer or zero length).");
        }
        currentHeaderValue.assign(at, length); // Log the operation
        if (currentHeaderField == "Content-Length") {
            response.body.reserve(std::stoi(currentHeaderValue));  // Pre-allocate space for body
        }
        response.headers.emplace_back(currentHeaderField, currentHeaderValue); // Log the header pair
        currentHeaderField.clear();
        return 0;
    });

    parser.onBody([&](const char* at, size_t length) {
        if (at == nullptr || length == 0) {
            throw exception(http_error::INVALID_BODY, "Invalid body data (null pointer or zero length).");
        }

        try {
            response.body.append(at, length);
        } catch (const std::exception& e) {
            throw exception(http_error::INTERNAL_ERROR,
                    "Failed to append body data: " + std::string(e.what()));
        }
        return 0;
    });

    parser.onStatus([&](const char* at, size_t length) {
        if (at == nullptr || length == 0) {
            throw exception(http_error::INVALID_STATUS, "Invalid status line (null pointer or zero length).");
        }
        response.status.message.assign(at, length); // Extract the status line
        return 0;
    });

    parser.onMessageComplete([&]() {
        response.status.code = parser.statusCode();
        if (!currentHeaderField.empty()) {
            response.headers.emplace_back(currentHeaderField, currentHeaderValue);
        }

        return 0;
    });

    parse(&parser);
    return response;
}

void requests::headers(const std::unordered_map<std::string, std::string>& headers) {
    // merge the headers with the existing headers
    for (auto& [key, value] : headers) {
        headers_[key] = value;
    }
}

void requests::header(const std::string& key, const std::string& value) {
    headers_[key] = value;
}

std::unordered_map<std::string, std::string> requests::headers() const {
    return headers_;
}

response requests::get(const std::string& path) {
    std::ostringstream data;
    data << "GET " << path << " HTTP/1.1\r\n";
    data << "Host: " << host_ << "\r\n";
    for (const auto& [key, value] : headers_) {
        data << key << ": " << value << "\r\n";
    }
    data << "\r\n";
    return send(data.str());
}


response requests::post(const std::string& path, const std::string& body) {
    std::ostringstream data;
    data << "POST " << path << " HTTP/1.1\r\n";
    data << "Host: " << host_ << "\r\n";
    data << "Content-Length: " << body.size() << "\r\n";
    for (const auto& [key, value] : headers_) {
        data << key << ": " << value << "\r\n";
    }
    data << "\r\n";
    data << body;
    return send(data.str());
}

std::ostream& operator<<(std::ostream& os, const http_status& status) {
    os << "HTTP " << status.code << " " << status.message;
    return os;
}

} // namespace htpp
