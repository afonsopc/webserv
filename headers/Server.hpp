#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Json;

class Server
{
public:
    Server(const Json &config);
    int getPort(void) const;
    std::string getHost(void) const;

private:
    int port;
    std::string host;
};

#endif
