#include "Server.hpp"
#include "HashMap.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

Server::Server(const HashMap &config)
	: fd(-1), port(config.get("port").asInt()), host(config.get("host").asString())
{
	std::vector<HashMapValue> routesArray = config.get("servers").asArray();
	for (std::vector<HashMapValue>::const_iterator it = routesArray.begin(); it != routesArray.end(); ++it)
		routes.push_back(Route(it->asHashMap()));
}

int Server::getPort(void) const { return port; }
std::string Server::getHost(void) const { return host; }
int Server::getFd(void) const { return fd; }
void Server::setFd(int fd) { this->fd = fd; }

bool Server::createSocket(void)
{
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		std::cerr << "Failed to create socket for server on " << host << ":" << port << std::endl;
		return false;
	}

	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "Failed to set socket options for server on " << host << ":" << port << std::endl;
		close(fd);
		fd = -1;
		return false;
	}

	return true;
}

bool Server::bindAndListen(void)
{
	if (fd < 0)
	{
		std::cerr << "Cannot bind: socket not created for server on " << host << ":" << port << std::endl;
		return false;
	}

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		std::cerr << "Bind failed on port " << port << std::endl;
		return false;
	}

	if (listen(fd, 128) < 0)
	{
		std::cerr << "Listen failed on port " << port << std::endl;
		return false;
	}

	return true;
}

void Server::closeSocket(void)
{
	if (fd >= 0)
		close(fd);
	fd = -1;
}

Response *Server::handleRequest(Request &req)
{
	Response::e_status status = Response::OK;
	std::string body = "ola DESCONHECIDO (ANONYMO :O) :)\n";
	if (req.getHeaders().get("Host").isString())
		body = "ola " + req.getHeaders().get("Host").asString() + req.getPath() + " :)\n";
	Http::e_version version = Http::HTTP_1_1;
	HashMap headers = HashMap();
	return (new Response(version, status, headers, body));
}
