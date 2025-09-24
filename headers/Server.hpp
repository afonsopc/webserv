#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include "Response.hpp"
#include "Request.hpp"

class HashMap;

class Server
{
public:
	Server(const HashMap &config);
	int getPort(void) const;
	std::string getHost(void) const;
	int getFd(void) const;
	void setFd(int fd);

	bool createSocket(void);
	bool bindAndListen(void);
	void closeSocket(void);
	Response *handleRequest(Request &req);

private:
	int fd;
	int port;
	std::string host;
};

#endif
