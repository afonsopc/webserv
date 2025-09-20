#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

#include "HashMap.hpp"
#include "queue.h"
#include "Server.hpp"
#include "WebServ.hpp"
#include "Http.hpp"

std::vector<Server> parseConfig(const char *config_file_name)
{
	std::ifstream file(config_file_name);
	std::ostringstream buffer;
	buffer << file.rdbuf();
	std::string configContent = buffer.str();
	HashMap serverConfig1(configContent);
	std::vector<Server> servers;
	std::vector<HashMapValue> serverArray = serverConfig1.get("servers").asArray();
	for (std::vector<HashMapValue>::const_iterator it = serverArray.begin(); it != serverArray.end(); ++it)
		servers.push_back(Server(it->asObject()));
	return servers;
}

void request(int client_fd, char *buffer, size_t bytes_read)
{
	std::string request_str(buffer, bytes_read);
	Request req(request_str);
	std::cout << "Host: " << req.getHeaders().get("Host").asString() << std::endl;
	write(client_fd, "HTTP/1.1 200 OK\r\n\r\nHello World", 30);
}

void serverLoop(int port)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
		return (std::cerr << "Bind failed" << std::endl, void(0));
	listen(server_fd, 128);
	setNonBlocking(server_fd);
	start_event_loop(server_fd, request);
	close(server_fd);
}

int main(int argc, char **argv)
{
	if (argc < 2)
		return (std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl, 1);
	std::vector<Server> servers = parseConfig(argv[1]);
	WebServ webserv(servers);
	serverLoop(3000);
}
