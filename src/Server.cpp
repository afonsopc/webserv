#include "Server.hpp"
#include "HashMap.hpp"
#include "Socket.hpp"
#include <iostream>

Server::~Server() { delete (socket); }
int Server::getPort(void) const { return (socket->getPort()); }
std::string Server::getHost(void) const { return (socket->getHost()); }
Socket &Server::getSocket(void) { return (*socket); }
const Socket &Server::getSocket(void) const { return (*socket); }

Server::Server(const HashMap &config)
	: socket(new Socket(config.get("host").asString(), config.get("port").asInt()))
{
	std::vector<HashMapValue> routesArray = config.get("routes").asArray();
	for (std::vector<HashMapValue>::const_iterator it = routesArray.begin(); it != routesArray.end(); ++it)
	{
		std::cout << "  Loading route #" << (routes.size() + 1) << " for path: " << it->asHashMap().get("path").asString() << std::endl;
		routes.push_back(Route(it->asHashMap()));
	}
}

bool Server::initialize(void)
{
	if (!socket->create())
	{
		std::cerr << "Failed to create socket for server on " << socket->getHost() << ":" << socket->getPort() << std::endl;
		return (false);
	}
	if (!socket->bindAndListen())
	{
		std::cerr << "Failed to bind/listen for server on " << socket->getHost() << ":" << socket->getPort() << std::endl;
		socket->close();
		return (false);
	}
	if (!socket->setNonBlocking())
	{
		std::cerr << "Failed to set non-blocking for server on " << socket->getHost() << ":" << socket->getPort() << std::endl;
		socket->close();
		return (false);
	}
	return (true);
}

Response *Server::handleRequest(Request &req)
{
	for (size_t i = 0; i < routes.size(); ++i)
		if (routes[i].matches(req))
		{
			std::cout << "Route matched: " << routes[i].getPath() << std::endl;
			return (routes[i].handleRequest(req));
		}
	int status = 404;
	std::string body = "404 Not Found\n";
	Http::e_version version = Http::HTTP_1_1;
	HashMap headers = HashMap();
	return (new Response(version, status, headers, body));
}
