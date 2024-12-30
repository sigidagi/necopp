#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace neco {

class http_parser;

struct http_status {
    int code = 0;               // HTTP status code (e.g., 200, 404)
    std::string message;        // HTTP status message (e.g., "OK", "Not Found")
};

std::ostream& operator<<(std::ostream& os, const http_status& status);

struct response {
    http_status status;
    std::vector<std::pair<std::string, std::string>> headers;
    std::string body;  // Raw response body
};

class requests {
public:
    explicit requests(const std::string& endpoint);
    ~requests();

    void headers(const std::unordered_map<std::string, std::string>& headers);
    std::unordered_map<std::string, std::string> headers() const;
    void header(const std::string& key, const std::string& value);

    response get(const std::string& path);
    response post(const std::string& path, const std::string& body);
private:
    void parse(http_parser* parser);
    response send(const std::string& data);

    std::string host_ = "localhost";
    std::string port_ = "80";
    int sock_ = -1;

    std::unordered_map<std::string, std::string> headers_;
};

}  // namespace neco 
