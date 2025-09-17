#include "Uri.hpp"

Uri::Uri(const std::string &uri)
{
    (void)uri; // Suppress unused parameter warning
    // TODO: Implement URI parsing
    port = 80;
    scheme = "http";
    host = "localhost";
    path = "/";
}

std::string Uri::getScheme() const
{
    return scheme;
}

std::string Uri::getHost() const
{
    return host;
}

int Uri::getPort() const
{
    return port;
}

std::string Uri::getPath() const
{
    return path;
}

std::map<std::string, std::string> Uri::getQueryParams() const
{
    return queryParams;
}