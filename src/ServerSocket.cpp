#include "ServerSocket.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <stdexcept>

ServerSocket::ServerSocket(int port) : server_fd(-1), addrlen(sizeof(address))
{
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Socket creation failed");

	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
		throw std::runtime_error("Setsockopt failed");

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	std::cout << "ServerSocket created successfully" << std::endl;
}

ServerSocket::~ServerSocket()
{
	closeSocket();
}

int ServerSocket::getSocketFd() const
{
	return server_fd;
}

void ServerSocket::closeSocket()
{
	if (server_fd != -1)
	{
		close(server_fd);
		server_fd = -1;
	}
}

bool ServerSocket::isValid() const
{
	return server_fd != -1;
}

void ServerSocket::bind()
{
	if (::bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
		throw std::runtime_error("Bind failed");
	std::cout << "Socket bound to port " << ntohs(address.sin_port) << std::endl;
}

void ServerSocket::listen(int backlog)
{
	if (::listen(server_fd, backlog) < 0)
		throw std::runtime_error("Listen failed");
	std::cout << "Server listening for connections..." << std::endl;
}

int ServerSocket::accept()
{
	int new_socket;
	if ((new_socket = ::accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
		throw std::runtime_error("Accept failed");
	return new_socket;
}