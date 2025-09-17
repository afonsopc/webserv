#include "ClientSocket.hpp"
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>

ClientSocket::ClientSocket(int fd) : client_fd(fd) {}

ClientSocket::~ClientSocket()
{
	closeSocket();
}

int ClientSocket::getSocketFd() const
{
	return client_fd;
}

void ClientSocket::closeSocket()
{
	if (client_fd != -1)
	{
		close(client_fd);
		client_fd = -1;
	}
}

bool ClientSocket::isValid() const
{
	return client_fd != -1;
}

std::string ClientSocket::readRequest()
{
	char buffer[BUFFER_SIZE] = {0};
	int valread = read(client_fd, buffer, BUFFER_SIZE);
	if (valread < 0)
		throw std::runtime_error("Read failed");
	request_buffer = std::string(buffer, valread);
	return request_buffer;
}

void ClientSocket::sendResponse(const std::string &response)
{
	send(client_fd, response.c_str(), response.length(), 0);
}

const std::string &ClientSocket::getRequest() const
{
	return request_buffer;
}