#include "Server.hpp"
#include "HashMap.hpp"
#include "Socket.hpp"
#include <iostream>

Server::~Server() { delete (socket); }
int Server::getPort(void) const { return (socket->getPort()); }
std::string Server::getHost(void) const { return (socket->getHost()); }
Socket &Server::getSocket(void) { return (*socket); }
const Socket &Server::getSocket(void) const { return (*socket); }
const std::map<std::string, std::string> &Server::getExtensions(void) const { return extensions; }

Server::Server(const HashMap &config)
	: socket(new Socket(config.get("host").asString(), config.get("port").asInt()))
{
	if (config.has("extensions"))
	{
		std::vector<HashMapValue> extensionsArray = config.get("extensions").asArray();
		for (std::vector<HashMapValue>::const_iterator it = extensionsArray.begin(); it != extensionsArray.end(); ++it)
		{
			HashMap extMap = it->asHashMap();
			std::string ext = extMap.get("ext").asString();
			std::string exec = extMap.get("exec").asString();
			extensions[ext] = exec;
		}
	}
	std::vector<HashMapValue> routesArray = config.get("routes").asArray();
	for (std::vector<HashMapValue>::const_iterator it = routesArray.begin(); it != routesArray.end(); ++it)
	{
		std::cout << "  Loading route #" << (routes.size() + 1) << " for path: " << it->asHashMap().get("path").asString() << std::endl;
		routes.push_back(Route(it->asHashMap(), extensions));
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
	if (req.getVersion() == Http::HTTP_1_1)
	{
		if (!req.getHeaders().get("Host").isString())
			return (new Response(Http::HTTP_1_1, 400, HashMap(), "400 Bad Request: Missing Host header\n"));
		std::string host_header = req.getHeaders().get("Host").asString();
		size_t colon_pos = host_header.find(':');
		std::string request_host = host_header.substr(0, colon_pos);
		std::string server_host = getHost();
		if (request_host != server_host)
			return (new Response(Http::HTTP_1_1, 400, HashMap(), "400 Bad Request: Host mismatch\n"));
	}
	for (size_t i = 0; i < routes.size(); ++i)
		if (routes[i].matches(req))
		{
			std::cout << "Route matched: " << routes[i].getPath() << std::endl;
			return (routes[i].handleRequest(req));
		}
	return (new Response(Http::HTTP_1_1, 404, HashMap(), "404 Not Found\n"));
}
