#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>

#include "Json.hpp"
#include "Server.hpp"
#include "WebServ.hpp"

std::vector<Server> parseConfig(std::ifstream &config_file)
{
    std::string configContent((std::istreambuf_iterator<char>(config_file)), std::istreambuf_iterator<char>());
    Json serverConfig1(configContent);
    std::vector<Server> servers;
    std::vector<JsonValue> serverArray = serverConfig1.get("servers").asArray();
    for (std::vector<JsonValue>::const_iterator it = serverArray.begin(); it != serverArray.end(); ++it)
        servers.push_back(Server(it->asObject()));
    return servers;
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return (std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl, 1);
    std::ifstream configFile(argv[1]);
    if (!configFile)
        return (std::cerr << "Failed to open config file: " << argv[1] << std::endl, 1);
    std::vector<Server> servers = parseConfig(configFile);
    configFile.close();
    WebServ webserv(servers);
    std::cout << "Hello, WebServ!" << std::endl;
}

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
