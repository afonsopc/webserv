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

static bool determineKeepAlive(const Request &req);
static void sendResponse(int client_fd, Response *res, bool keep_alive);
static size_t extractContentLength(const std::string &headers_part);

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
		res = new Response(Http::HTTP_1_1, 500, HashMap(), "500 Internal Server Error\n");
	bool keep_alive = determineKeepAlive(req);
	sendResponse(client_fd, res, keep_alive);
	delete res;
	return (keep_alive);
}

static bool determineKeepAlive(const Request &req)
{
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
	return keep_alive;
}

static void sendResponse(int client_fd, Response *res, bool keep_alive)
{
	res->setHeader("Connection", keep_alive ? "keep-alive" : "close");

	if (!res->getHeaders().get("Content-Length").isString())
	{
		std::ostringstream oss;
		oss << res->getBody().length();
		res->setHeader("Content-Length", oss.str());
	}

	std::string response_str = res->stringify();
	send(client_fd, response_str.c_str(), response_str.size(), 0);
}

bool WebServ::handleClientData(int client_fd, char *buffer, size_t bytes_read)
{
	client_buffers[client_fd].append(buffer, bytes_read);

	std::string &request_buffer = client_buffers[client_fd];

	size_t header_end = request_buffer.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return (true);

	std::string headers_part = request_buffer.substr(0, header_end + 4);
	size_t content_length = extractContentLength(headers_part);
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

static size_t extractContentLength(const std::string &headers_part)
{
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
	return content_length;
}

void WebServ::loop(void)
{
	setupSignals();
	std::vector<int> server_fds = initializeServers();
	if (server_fds.empty())
		return;

	int epoll_fd = setupEpoll(server_fds);
	runEventLoop(epoll_fd, server_fds);
	cleanup(epoll_fd);
}

void WebServ::setupSignals(void)
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	std::cout << "\nSignal handlers configured. Ctrl+C will work now." << std::endl;
}

std::vector<int> WebServ::initializeServers(void)
{
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
		std::cerr << "No servers could be started. Exiting..." << std::endl;
	else
		std::cout << "Starting event loop. Press Ctrl+C to stop." << std::endl;
	return server_fds;
}

int WebServ::setupEpoll(const std::vector<int> &server_fds)
{
	int epoll_fd = epoll_create1(0);
	struct epoll_event event;
	for (size_t i = 0; i < server_fds.size(); ++i)
	{
		event.events = EPOLLIN;
		event.data.fd = server_fds[i];
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fds[i], &event);
	}
	return epoll_fd;
}

void WebServ::runEventLoop(int epoll_fd, const std::vector<int> &server_fds)
{
	struct epoll_event events[64];
	while (!g_shutdown)
	{
		int nfds = epoll_wait(epoll_fd, events, 64, 1000);
		if (nfds == -1 || nfds == 0)
			continue;
		for (int i = 0; i < nfds; i++)
			handleEpollEvent(epoll_fd, events[i], server_fds);
	}
}

void WebServ::handleEpollEvent(int epoll_fd, struct epoll_event event, const std::vector<int> &server_fds)
{
	bool is_server_socket = false;
	for (size_t j = 0; j < server_fds.size(); ++j)
	{
		if (event.data.fd == server_fds[j])
		{
			is_server_socket = true;
			break;
		}
	}

	if (is_server_socket)
		handleServerSocket(event.data.fd, epoll_fd);
	else
		handleClientSocket(event.data.fd, epoll_fd);
}

void WebServ::handleServerSocket(int server_fd, int epoll_fd)
{
	Server *server = getServerFromFd(server_fd);
	int client_fd = server->getSocket().acceptConnection();
	if (client_fd >= 0)
	{
		setNonBlocking(client_fd);
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = client_fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
	}
}

void WebServ::handleClientSocket(int client_fd, int epoll_fd)
{
	char buffer[4096];
	ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
	if (bytes > 0)
	{
		buffer[bytes] = '\0';
		bool keep_alive = handleClientData(client_fd, buffer, bytes);

		if (!keep_alive)
		{
			client_buffers.erase(client_fd);
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
			close(client_fd);
		}
	}
	else if (bytes <= 0)
	{
		client_buffers.erase(client_fd);
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
		close(client_fd);
	}
}

void WebServ::cleanup(int epoll_fd)
{
	std::cout << "Shutting down servers..." << std::endl;
	client_buffers.clear();
	close(epoll_fd);
	for (size_t i = 0; i < servers.size(); ++i)
		servers[i]->getSocket().close();
	std::cout << "Server shutdown complete." << std::endl;
}