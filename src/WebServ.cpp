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
	std::cout << "Host: " << req.getHeaders().get("Host").asString() << std::endl;
	write(client_fd, "HTTP/1.1 200 OK\r\n\r\nHello World", 30);
}

void WebServ::loop(void)
{
	std::cout << (servers.begin() == servers.end() ? "no server" : "server") << std::endl;
	for (std::vector<Server>::const_iterator it = servers.begin(); it != servers.end(); ++it)
	{
		std::cout << "Starting server on " << it->getHost() << ":" << it->getPort() << std::endl;
		int server_fd = socket(AF_INET, SOCK_STREAM, 0);
		int opt = 1;
		setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

		struct sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_port = htons(it->getPort());
		address.sin_addr.s_addr = INADDR_ANY;

		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
			return (std::cerr << "Bind failed on port" << it->getPort() << std::endl, void(0));
		listen(server_fd, 128);
		setNonBlocking(server_fd);
		start_event_loop(server_fd, request);
		close(server_fd);
	}
}