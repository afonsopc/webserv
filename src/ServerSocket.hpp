#ifndef SERVER_SOCKET_HPP
#define SERVER_SOCKET_HPP

#include "Socket.hpp"
#include <sys/socket.h>
#include <netinet/in.h>

class ServerSocket : public Socket
{
private:
	int server_fd;
	struct sockaddr_in address;
	int addrlen;

public:
	ServerSocket(int port);
	~ServerSocket();

	int getSocketFd() const;
	void closeSocket();
	bool isValid() const;

	void bind();
	void listen(int backlog = 10);
	int accept();
};

#endif