#include "WebServ.hpp"
#include "Socket.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
#include <cstdlib>
#include <cstring>
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

WebServ::WebServ(const HashMap &config)
{
	std::vector<HashMapValue> serverArray = config.get("servers").asArray();
	for (std::vector<HashMapValue>::const_iterator it = serverArray.begin(); it != serverArray.end(); ++it)
	{
		std::cout << "Loading server #" << (servers.size() + 1) << " for: " << it->asHashMap().get("host").asString() << ":" << it->asHashMap().get("port").asInt() << std::endl;
		servers.push_back(new Server(it->asHashMap()));
	}
}

WebServ::~WebServ()
{
	for (size_t i = 0; i < servers.size(); ++i)
		delete (servers[i]);
}

Server *WebServ::getServerFromFd(int fd)
{
	for (size_t i = 0; i < servers.size(); ++i)
		if (servers[i]->getSocket().getFd() == fd)
			return (servers[i]);
	throw(std::runtime_error("Server not found for given file descriptor"));
}

bool WebServ::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return (false);
	return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1);
}

Server *WebServ::getServerFromClientFd(int client_fd)
{
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	if (getsockname(client_fd, (struct sockaddr *)&addr, &addr_len) == 0)
	{
		int port = ntohs(addr.sin_port);

		for (size_t i = 0; i < servers.size(); ++i)
			if (servers[i]->getPort() == port)
				return (servers[i]);
	}

	throw(std::runtime_error("No server found for client connection"));
}

bool WebServ::processRequest(int client_fd, const std::string &complete_request)
{
	Server *server = getServerFromClientFd(client_fd);
	Request req(complete_request);
	Response *res = server->handleRequest(req);

	if (!res)
	{
		Response::e_status status = Response::INTERNAL_SERVER_ERROR;
		std::string body = "500 Internal Server Error\n";
		Http::e_version version = Http::HTTP_1_1;
		HashMap headers = HashMap();
		res = new Response(version, status, headers, body);
	}

	bool keep_alive = false;
	if (req.getVersion() == Http::HTTP_1_1)
	{
		keep_alive = true;
		if (req.getHeaders().get("Connection").isString())
		{
			std::string connection = req.getHeaders().get("Connection").asString();
			if (connection == "close")
				keep_alive = false;
		}
	}
	else if (req.getVersion() == Http::HTTP_1_0)
	{
		keep_alive = false;
		if (req.getHeaders().get("Connection").isString())
		{
			std::string connection = req.getHeaders().get("Connection").asString();
			if (connection == "keep-alive")
				keep_alive = true;
		}
	}

	res->setHeader("Connection", keep_alive ? "keep-alive" : "close");

	if (!res->getHeaders().get("Content-Length").isString())
	{
		std::ostringstream oss;
		oss << res->getBody().length();
		res->setHeader("Content-Length", oss.str());
	}

	std::string response_str = res->stringify();
	write(client_fd, response_str.c_str(), response_str.size());
	delete res;
	return (keep_alive);
}

bool WebServ::handleClientData(int client_fd, char *buffer, size_t bytes_read)
{
	client_buffers[client_fd].append(buffer, bytes_read);

	std::string &request_buffer = client_buffers[client_fd];

	size_t header_end = request_buffer.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return (true);

	std::string headers_part = request_buffer.substr(0, header_end + 4);

	size_t content_length = 0;
	size_t cl_pos = headers_part.find("Content-Length:");
	if (cl_pos == std::string::npos)
		cl_pos = headers_part.find("content-length:");

	if (cl_pos != std::string::npos)
	{
		size_t cl_start = headers_part.find(':', cl_pos) + 1;
		size_t cl_end = headers_part.find('\r', cl_start);
		if (cl_end != std::string::npos)
		{
			std::string cl_str = headers_part.substr(cl_start, cl_end - cl_start);
			cl_str.erase(0, cl_str.find_first_not_of(" \t"));
			cl_str.erase(cl_str.find_last_not_of(" \t") + 1);
			content_length = std::atol(cl_str.c_str());
		}
	}

	size_t total_expected = header_end + 4 + content_length;
	if (request_buffer.length() < total_expected)
		return (true);

	std::string complete_request = request_buffer.substr(0, total_expected);
	bool keep_alive = processRequest(client_fd, complete_request);

	request_buffer.erase(0, total_expected);

	if (!keep_alive || request_buffer.empty())
		client_buffers.erase(client_fd);
	return (keep_alive);
}

void WebServ::loop(void)
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	std::cout << "\nSignal handlers configured. Ctrl+C will work now." << std::endl;

	std::vector<int> server_fds;

	for (size_t i = 0; i < servers.size(); ++i)
	{
		std::cout << "Starting server on http://" << servers[i]->getHost() << ":" << servers[i]->getPort() << std::endl;

		if (!servers[i]->initialize())
		{
			std::cerr << "Failed to initialize server on " << servers[i]->getHost() << ":" << servers[i]->getPort() << std::endl;
			continue;
		}

		server_fds.push_back(servers[i]->getSocket().getFd());
	}

	if (server_fds.empty())
	{
		std::cerr << "No servers could be started. Exiting..." << std::endl;
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
		int nfds = epoll_wait(epoll_fd, events, 64, 1000);
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
				Server *server = getServerFromFd(events[i].data.fd);
				int client_fd = server->getSocket().acceptConnection();
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
					bool keep_alive = handleClientData(events[i].data.fd, buffer, bytes);

					if (!keep_alive)
					{
						client_buffers.erase(events[i].data.fd);
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
						close(events[i].data.fd);
					}
				}
				else if (bytes <= 0)
				{
					client_buffers.erase(events[i].data.fd);
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					close(events[i].data.fd);
				}
			}
		}
	}

	std::cout << "Shutting down servers..." << std::endl;

	client_buffers.clear();

	close(epoll_fd);

	for (size_t i = 0; i < servers.size(); ++i)
		servers[i]->getSocket().close();

	std::cout << "Server shutdown complete." << std::endl;
}