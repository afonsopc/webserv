#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>

#include "HashMap.hpp"
#include "Server.hpp"
#include "WebServ.hpp"

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

int main(int argc, char **argv)
{
    if (argc < 2)
        return (std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl, 1);
    std::vector<Server> servers = parseConfig(argv[1]);
    WebServ webserv(servers);
    std::cout << "Hello, WebServ!" << std::endl;
}
