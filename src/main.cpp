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
