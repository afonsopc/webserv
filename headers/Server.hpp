#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class HashMap;

class Server
{
public:
    Server(const HashMap &config);
    int getPort(void) const;
    std::string getHost(void) const;

private:
    int port;
    std::string host;
};

#endif
