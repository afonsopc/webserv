#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>

#include "Json.hpp"
#include "Server.hpp"
#include "WebServ.hpp"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }
    std::ifstream configFile(argv[1]);
    if (!configFile)
    {
        std::cerr << "Failed to open config file: " << argv[1] << std::endl;
        return 1;
    }
    std::string configContent((std::istreambuf_iterator<char>(configFile)), std::istreambuf_iterator<char>());
    configFile.close();
    Json serverConfig1(configContent);
    std::vector<Server> servers;
    std::vector<JsonValue> serverArray = serverConfig1.get("servers").asArray();
    for (std::vector<JsonValue>::const_iterator it = serverArray.begin(); it != serverArray.end(); ++it)
        servers.push_back(Server(it->asObject()));
    WebServ webserv(servers);
    std::cout << "Hello, WebServ!" << std::endl;
}
