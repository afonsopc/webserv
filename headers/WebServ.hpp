#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <vector>
#include "Server.hpp"

class WebServ
{
public:
	WebServ(const std::vector<Server> &servers);
	void loop(void);
	Server &getServerFromFd(int fd);
	Server &getServerFromClientFd(int client_fd);
	void request(int client_fd, char *buffer, size_t bytes_read);
	int setNonBlocking(int fd);

private:
	std::vector<Server> servers;
};

#endif
