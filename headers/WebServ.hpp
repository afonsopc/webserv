#ifndef WEBSERV_HPP
#define WEBSERV_HPP

int *argc_singleton(void);
char ***argv_singleton(void);
char ***envp_singleton(void);
class WebServ;
WebServ **webserv_singleton(void);

#include <vector>
#include <map>
#include <string>
#include <ctime>
#include <sys/types.h>
#include "Server.hpp"

struct PendingCgi
{
	int client_fd;
	int pipe_fd;
	pid_t pid;
	std::string output;
	time_t start_time;
	Request request;

	PendingCgi(int cf, int pf, pid_t p, const Request &r)
		: client_fd(cf), pipe_fd(pf), pid(p), start_time(time(NULL)), request(r) {}
};

struct PendingWrite
{
	std::string data;
	size_t bytes_sent;
	bool keep_alive;

	PendingWrite(const std::string &d, bool ka)
		: data(d), bytes_sent(0), keep_alive(ka) {}
};

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
	void handleCgiData(int pipe_fd);
	int startAsyncCgi(int client_fd, Request &req, const std::string &interpreter, const std::string &scriptPath);
	void queueWrite(int client_fd, const std::string &data, bool keep_alive);
	void handleWriteReady(int client_fd);

private:
	std::vector<Server *> servers;
	std::map<int, std::string> client_buffers;
	std::map<int, time_t> client_last_activity;
	std::map<int, PendingCgi> pending_cgis;
	std::map<int, PendingWrite> pending_writes;
	int epoll_fd;
};

#endif
