#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
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

	bool initialize(void);
	Response *handleRequest(Request &req);

private:
	Socket *socket;
	std::vector<Route> routes;
};

#endif
