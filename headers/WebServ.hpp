#ifndef WEBSERV_HPP
#define WEBSERV_HPP

int *argc_singleton(void);
char ***argv_singleton(void);
char ***envp_singleton(void);

#include <vector>
#include <map>
#include <string>
#include "Server.hpp"

class WebServ
{
public:
	WebServ(const HashMap &config);
	~WebServ();
	void loop(void);
	Server *getServerFromFd(int fd);
	Server *getServerFromClientFd(int client_fd);
	bool processRequest(int client_fd, const std::string &complete_request);
	bool handleClientData(int client_fd, char *buffer, size_t bytes_read);
	bool setNonBlocking(int fd);

private:
	std::vector<Server *> servers;
	std::map<int, std::string> client_buffers;
};

#endif
