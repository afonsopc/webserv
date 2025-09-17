#include "Server.hpp"
#include "HashMap.hpp"

Server::Server(const HashMap &config)
    : port(config.get("port").asInt()), host(config.get("host").asString())
{
}

int Server::getPort(void) const { return port; }
std::string Server::getHost(void) const { return host; }
