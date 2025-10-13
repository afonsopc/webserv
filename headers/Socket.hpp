#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <string>

class Socket
{
public:
    Socket(const std::string &host, int port);
    ~Socket();

    bool create(void);
    bool bindAndListen(void);
    int acceptConnection(void);
    void close(void);
    bool setNonBlocking(void);
    bool setReuseAddr(void);

    int getFd(void) const;
    int getPort(void) const;
    std::string getHost(void) const;
    bool isValid(void) const;

private:
    int fd;
    int port;
    std::string host;
};

#endif
