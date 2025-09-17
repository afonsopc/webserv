#include "Server.hpp"
#include "Json.hpp"

Server::Server(const Json &config)
    : port(config.get("port").asInt()), host(config.get("host").asString())
{
}

int Server::getPort(void) const { return port; }
std::string Server::getHost(void) const { return host; }
