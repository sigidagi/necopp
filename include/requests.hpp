#pragma once

#include "http.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace neco {

class requests : public http {
public:
    explicit requests(const std::string& endpoint);
    ~requests() = default;

    void headers(const std::unordered_map<std::string, std::string>& headers);
    std::unordered_map<std::string, std::string> headers() const;
    void header(const std::string& key, const std::string& value);

    response get(const std::string& path);
    response post(const std::string& path, const std::string& body);
   
private:
    std::string host_ = "localhost";
    std::string port_ = "80";

    std::unordered_map<std::string, std::string> headers_;
};

}  // namespace neco 
