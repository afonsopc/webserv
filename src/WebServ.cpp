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
#include <sys/wait.h>
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

WebServ::WebServ(const HashMap &config) : epoll_fd(-1)
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

	if (res->getStatus() == -1)
	{
		std::string interpreter = res->getHeaders().get("X-CGI-Interpreter").asString();
		std::string scriptPath = res->getHeaders().get("X-CGI-Script").asString();
		delete res;

		int pipe_fd = startAsyncCgi(client_fd, req, interpreter, scriptPath);
		if (pipe_fd < 0)
		{
			Response *error_res = new Response(Http::HTTP_1_1, 500, HashMap(), "CGI Error: Failed to start CGI\n");
			std::string response_str = error_res->stringify();
			queueWrite(client_fd, response_str, false);
			delete error_res;
			return (false);
		}

		return (true);
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
	res->setHeader("Access-Control-Allow-Origin", "*");
	res->setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD");
	res->setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");

	if (!res->getHeaders().get("Content-Length").isString())
	{
		std::ostringstream oss;
		oss << res->getBody().length();
		res->setHeader("Content-Length", oss.str());
	}

	std::string response_str = res->stringify();
	queueWrite(client_fd, response_str, keep_alive);
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

int WebServ::startAsyncCgi(int client_fd, Request &req, const std::string &interpreter, const std::string &scriptPath)
{
	int pipefd[2];
	if (pipe(pipefd) == -1)
		return (-1);

	pid_t pid = fork();
	if (pid == -1)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		return (-1);
	}

	if (pid == 0)
	{
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		std::string rawRequest = req.getRaw();
		char *args[] = {const_cast<char *>(interpreter.c_str()),
						const_cast<char *>(scriptPath.c_str()),
						const_cast<char *>(rawRequest.c_str()), NULL};
		execve(interpreter.c_str(), args, *envp_singleton());
		std::cerr << "CGI Error: Failed to execute " << interpreter << " with " << scriptPath << std::endl;
		exit(1);
	}
	else
	{
		close(pipefd[1]);
		setNonBlocking(pipefd[0]);
		pending_cgis.insert(std::make_pair(pipefd[0], PendingCgi(client_fd, pipefd[0], pid, req)));

		if (epoll_fd >= 0)
		{
			struct epoll_event event;
			event.events = EPOLLIN;
			event.data.fd = pipefd[0];
			epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipefd[0], &event);
		}

		return (pipefd[0]);
	}
}

void WebServ::handleCgiData(int pipe_fd)
{
	std::map<int, PendingCgi>::iterator it = pending_cgis.find(pipe_fd);
	if (it == pending_cgis.end())
		return;

	PendingCgi &cgi = it->second;
	char buffer[4096];
	ssize_t bytesRead = read(pipe_fd, buffer, sizeof(buffer));

	if (bytesRead > 0)
	{
		cgi.output.append(buffer, bytesRead);
	}
	else if (bytesRead == 0)
	{
		close(pipe_fd);
		int status;
		waitpid(cgi.pid, &status, 0);

		bool timedOut = (time(NULL) - cgi.start_time) > 5;

		if (timedOut)
		{
			std::string timeoutMsg = "CGI Error: Execution timed out after 5 seconds\n";
			Response *res = new Response(Http::HTTP_1_1, 504, HashMap(), timeoutMsg);
			std::string response_str = res->stringify();
			queueWrite(cgi.client_fd, response_str, false);
			delete res;
		}
		else
		{
			HashMap headers = HashMap();
			std::istringstream iss(cgi.output);
			std::string statusLine;
			std::getline(iss, statusLine);
			int http_status = 200;
			if (!statusLine.empty() && statusLine[statusLine.size() - 1] == '\r')
				statusLine.erase(statusLine.size() - 1);
			if (!statusLine.empty())
				http_status = std::atoi(statusLine.c_str());

			while (true)
			{
				std::string header;
				std::getline(iss, header);
				if (header.empty() || header == "\r")
					break;
				size_t colonPos = header.find(':');
				if (colonPos != std::string::npos)
				{
					std::string key = header.substr(0, colonPos);
					std::string value = header.substr(colonPos + 1);
					headers.set(key, value);
				}
			}
			std::ostringstream body;
			body << iss.rdbuf();

			Response *res = new Response(Http::HTTP_1_1, http_status, headers, body.str());

			bool keep_alive = false;
			if (cgi.request.getVersion() == Http::HTTP_1_1)
			{
				keep_alive = true;
				if (cgi.request.getHeaders().get("Connection").isString())
				{
					std::string connection = cgi.request.getHeaders().get("Connection").asString();
					if (connection == "close")
						keep_alive = false;
				}
			}
			res->setHeader("Connection", keep_alive ? "keep-alive" : "close");
			res->setHeader("Access-Control-Allow-Origin", "*");
			res->setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD");
			res->setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");

			if (!res->getHeaders().get("Content-Length").isString())
			{
				std::ostringstream oss;
				oss << res->getBody().length();
				res->setHeader("Content-Length", oss.str());
			}

			std::string response_str = res->stringify();
			queueWrite(cgi.client_fd, response_str, keep_alive);
			delete res;
		}

		pending_cgis.erase(it);
	}
	else if ((time(NULL) - cgi.start_time) > 5)
	{
		kill(cgi.pid, SIGKILL);
		close(pipe_fd);
		waitpid(cgi.pid, NULL, 0);

		std::string timeoutMsg = "CGI Error: Execution timed out after 5 seconds\n";
		Response *res = new Response(Http::HTTP_1_1, 504, HashMap(), timeoutMsg);
		std::string response_str = res->stringify();
		queueWrite(cgi.client_fd, response_str, false);
		delete res;

		pending_cgis.erase(it);
	}
}

void WebServ::queueWrite(int client_fd, const std::string &data, bool keep_alive)
{
	std::map<int, PendingWrite>::iterator it = pending_writes.find(client_fd);
	if (it != pending_writes.end())
	{
		it->second.data.append(data);
		return;
	}

	pending_writes.insert(std::make_pair(client_fd, PendingWrite(data, keep_alive)));

	if (epoll_fd >= 0)
	{
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLOUT;
		event.data.fd = client_fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &event);
	}
}

void WebServ::handleWriteReady(int client_fd)
{
	std::map<int, PendingWrite>::iterator it = pending_writes.find(client_fd);
	if (it == pending_writes.end())
		return;

	PendingWrite &pw = it->second;
	size_t remaining = pw.data.size() - pw.bytes_sent;

	ssize_t bytes_written = write(client_fd, pw.data.c_str() + pw.bytes_sent, remaining);

	if (bytes_written <= 0)
	{
		pending_writes.erase(it);
		client_buffers.erase(client_fd);
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
		close(client_fd);
		return;
	}

	pw.bytes_sent += bytes_written;

	if (pw.bytes_sent >= pw.data.size())
	{
		bool keep_alive = pw.keep_alive;
		pending_writes.erase(it);

		if (keep_alive)
		{
			struct epoll_event event;
			event.events = EPOLLIN;
			event.data.fd = client_fd;
			epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &event);
		}
		else
		{
			client_buffers.erase(client_fd);
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
			close(client_fd);
		}
	}
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
	epoll_fd = epoll_create1(0);
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
			bool is_cgi_pipe = false;

			for (size_t j = 0; j < server_fds.size(); ++j)
			{
				if (events[i].data.fd == server_fds[j])
				{
					is_server_socket = true;
					break;
				}
			}

			if (!is_server_socket && pending_cgis.find(events[i].data.fd) != pending_cgis.end())
				is_cgi_pipe = true;

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
			else if (is_cgi_pipe)
			{
				handleCgiData(events[i].data.fd);
				if (pending_cgis.find(events[i].data.fd) == pending_cgis.end())
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
			}
			else
			{
				if (events[i].events & EPOLLOUT)
					handleWriteReady(events[i].data.fd);

				if (events[i].events & EPOLLIN)
				{
					char buffer[4096];
					ssize_t bytes = recv(events[i].data.fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
					if (bytes > 0)
					{
						buffer[bytes] = '\0';
						bool keep_alive = handleClientData(events[i].data.fd, buffer, bytes);

						if (!keep_alive && pending_writes.find(events[i].data.fd) == pending_writes.end())
						{
							client_buffers.erase(events[i].data.fd);
							epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
							close(events[i].data.fd);
						}
					}
					else if (bytes <= 0)
					{
						client_buffers.erase(events[i].data.fd);
						pending_writes.erase(events[i].data.fd);
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
						close(events[i].data.fd);
					}
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