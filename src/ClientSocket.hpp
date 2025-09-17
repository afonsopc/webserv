#ifndef CLIENT_SOCKET_HPP
#define CLIENT_SOCKET_HPP

#include "Socket.hpp"
#include <string>

#define BUFFER_SIZE 1024

class ClientSocket : public Socket
{
private:
	int client_fd;
	std::string request_buffer;

public:
	ClientSocket(int fd);
	~ClientSocket();

	int getSocketFd() const;
	void closeSocket();
	bool isValid() const;

	std::string readRequest();
	void sendResponse(const std::string &response);
	const std::string &getRequest() const;
};

#endif