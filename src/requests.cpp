#include "error.hpp"
#include "requests.hpp"
#include "neco.h"
#include "http_parser.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <regex>
#include <functional>
#include <string_view>
#include <string>
#include <iostream>

namespace neco {

requests::requests(const std::string& url) : http() {
    
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
    send(data.str());  // http::send method
    return receive();  // http::receive method
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
    send(data.str());  // http::send method
    return receive();  // http::receive method
}


} // namespace htpp
