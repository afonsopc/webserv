#include "WebServ.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
#include <cstdlib>
#include "Response.hpp"
#include "Request.hpp"

volatile sig_atomic_t g_shutdown = 0;

void signal_handler(int signal)
{
	if (signal != SIGINT && signal != SIGTERM)
		return;
	std::cout << "\nReceived signal " << signal << ". Shutting down gracefully..." << std::endl;
	g_shutdown = 1;
}

WebServ::WebServ(const std::vector<Server> &servers)
	: servers(servers) {}

Server &WebServ::getServerFromFd(int fd)
{
	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
		if (it->getFd() == fd)
			return (*it);
	throw(std::runtime_error("Server not found for given file descriptor"));
}

int WebServ::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	return (fcntl(fd, F_SETFL, flags | O_NONBLOCK));
}

Server &WebServ::getServerFromClientFd(int client_fd)
{
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	if (getsockname(client_fd, (struct sockaddr *)&addr, &addr_len) == 0)
		for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
			if (it->getPort() == ntohs(addr.sin_port))
				return (*it);
	throw(std::runtime_error("No server found for client connection"));
}

void WebServ::request(int client_fd, char *buffer, size_t bytes_read)
{
	Server &server = getServerFromClientFd(client_fd);
	std::string request_str(buffer, bytes_read);
	Request req(request_str);
	Response *res = server.handleRequest(req);
	std::string response_str = res->stringify();
	write(client_fd, response_str.c_str(), response_str.size());
	delete res;
}

void WebServ::loop(void)
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	std::cout << "Signal handlers configured. Ctrl+C will work now." << std::endl;

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
		std::cerr << "No servers available to run. Exiting." << std::endl;
		return;
	}

	std::cout << "Starting event loop. Press Ctrl+C to stop." << std::endl;

	int epoll_fd = epoll_create1(0);
	struct epoll_event event, events[64];

	for (size_t i = 0; i < server_fds.size(); ++i)
	{
		event.events = EPOLLIN;
		event.data.fd = server_fds[i];
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fds[i], &event);
	}

	while (!g_shutdown)
	{
		int nfds = epoll_wait(epoll_fd, events, 64, 1000); // timeout 1 segundo
		if (nfds == -1 || nfds == 0)
			continue;
		for (int i = 0; i < nfds; i++)
		{
			bool is_server_socket = false;
			for (size_t j = 0; j < server_fds.size(); ++j)
			{
				if (events[i].data.fd == server_fds[j])
				{
					is_server_socket = true;
					break;
				}
			}

			if (is_server_socket)
			{
				int client_fd = accept(events[i].data.fd, NULL, NULL);
				if (client_fd >= 0)
				{
					setNonBlocking(client_fd);
					event.events = EPOLLIN;
					event.data.fd = client_fd;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
				}
			}
			else
			{
				char buffer[4096];
				ssize_t bytes = recv(events[i].data.fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
				if (bytes > 0)
				{
					buffer[bytes] = '\0';
					request(events[i].data.fd, buffer, bytes);
				}
				close(events[i].data.fd);
			}
		}
	}

	std::cout << "Shutting down servers..." << std::endl;
	close(epoll_fd);

	for (std::vector<Server>::iterator it = servers.begin(); it != servers.end(); ++it)
		it->closeSocket();

	std::cout << "Server shutdown complete." << std::endl;
}