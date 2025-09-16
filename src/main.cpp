#include <iostream>

class Server
{
public:
    Server(int port, const std::string &host) : port(port), host(host) {}
    int getPort() const { return (port); }
    std::string getHost() const { return (host); }

private:
    int port;
    std::string host;
};

class WebServ
{
public:
    WebServ(std::vector<Server> servers) : servers(servers) {}

private:
    std::vector<Server> servers;
};

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    Server server1(8080, "localhost");
    Server server2(9090, "localhost");
    std::vector<Server> servers;
    servers.push_back(server1);
    servers.push_back(server2);
    WebServ webserv(servers);
    std::cout << "Hello, WebServ!" << std::endl;
}

// int main(int argc, char **argv)
// {
// 	if (argc != 2)
// 	{
// 		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
// 		return 1;
// 	}

// 	try
// 	{
// 		Config config = parseConfig(argv[1]);

// 		ServerSocket server(config.port);

// 		server.bind();

// 		server.listen(config.backlog);

// 		std::cout << "Web server started on port " << config.port << std::endl;
// 		std::cout << "Press Ctrl+C to stop the server" << std::endl;

// 		while (true)
// 		{
// 			int client_fd = server.accept();
// 			std::cout << "New client connected" << std::endl;

// 			ClientSocket client(client_fd);

// 			std::string request_str = client.readRequest();
// 			std::cout << "Received request:\n"
// 					  << request_str << std::endl;

// 			HttpRequest request(request_str);

// 			HttpResponse response;
// 			response.setHeader("Content-Type", "text/html");
// 			response.setBody("Hello, World!");

// 			client.sendResponse(response.toString());
// 			std::cout << "Response sent to client" << std::endl;
// 		}
// 	}
// 	catch (const std::exception &e)
// 	{
// 		std::cerr << "Error: " << e.what() << std::endl;
// 		return 1;
// 	}

// 	return 0;
// }