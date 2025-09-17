#ifndef URI_HPP
#define URI_HPP

#include <string>
#include <map>
#include "HashMap.hpp"

class Uri
{
    std::string scheme;
    std::string host;
    int port;
    std::string path;
    std::map<std::string, std::string> queryParams;

public:
    Uri(const std::string &uri);
    std::string getScheme() const;
    std::string getHost() const;
    int getPort() const;
    std::string getPath() const;
    std::map<std::string, std::string> getQueryParams() const;
};

#endif
