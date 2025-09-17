#include "src/ServerSocket.hpp"
#include "src/ClientSocket.hpp"
#include "src/HttpRequest.hpp"
#include "src/HttpResponse.hpp"
#include "src/Config.hpp"
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		return 1;
	}

	try
	{
		Config config = parseConfig(argv[1]);

		ServerSocket server(config.port);

		server.bind();

		server.listen(config.backlog);

		std::cout << "Web server started on port " << config.port << std::endl;
		std::cout << "Press Ctrl+C to stop the server" << std::endl;

		while (true)
		{
			int client_fd = server.accept();
			std::cout << "New client connected" << std::endl;

			ClientSocket client(client_fd);

			std::string request_str = client.readRequest();
			std::cout << "Received request:\n"
					  << request_str << std::endl;

			HttpRequest request(request_str);

			HttpResponse response;
			response.setHeader("Content-Type", "text/html");
			response.setBody("Hello, World!");

			client.sendResponse(response.toString());
			std::cout << "Response sent to client" << std::endl;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}