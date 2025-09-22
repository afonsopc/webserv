#include "WebServ.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include "queue.h"
#include "Http.hpp"

WebServ::WebServ(const std::vector<Server> &servers)
	: servers(servers) {}

static void request(int client_fd, char *buffer, size_t bytes_read)
{
	std::string request_str(buffer, bytes_read);
	Request req(request_str);
	write(client_fd, "HTTP/1.1 200 OK\r\n\r\nHello World", 30);
}

void WebServ::loop(void)
{
	std::vector<int> server_fds;

	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
	{
		std::cout << "Starting server on http://" << it->getHost() << ":" << it->getPort() << std::endl;

		if (!it->createSocket())
		{
			std::cerr << "Failed to create socket for server on port " << it->getPort() << std::endl;
			continue;
		}

		if (!it->bindAndListen())
		{
			std::cerr << "Failed to bind/listen for server on port " << it->getPort() << std::endl;
			it->closeSocket();
			continue;
		}

		setNonBlocking(it->getFd());
		server_fds.push_back(it->getFd());
	}

	if (server_fds.empty())
	{
		std::cerr << "No servers could be started" << std::endl;
		return;
	}

	if (!server_fds.empty())
	{
		int *fds_array = new int[server_fds.size()];
		for (size_t i = 0; i < server_fds.size(); ++i)
			fds_array[i] = server_fds[i];
		start_multi_server_event_loop(fds_array, server_fds.size(), request);
		delete[] fds_array;
	}

	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
		it->closeSocket();
}