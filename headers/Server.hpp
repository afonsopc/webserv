#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include "Response.hpp"
#include "Request.hpp"
#include "Route.hpp"

class HashMap;
class Socket;

class Server
{
public:
	Server(const HashMap &config);
	~Server();

	int getPort(void) const;
	std::string getHost(void) const;
	Socket &getSocket(void);
	const Socket &getSocket(void) const;
	const std::map<std::string, std::string> &getExtensions(void) const;

	bool initialize(void);
	Response *handleRequest(Request &req);

private:
	Socket *socket;
	std::vector<Route> routes;
	std::map<std::string, std::string> extensions;
};

#endif
