#include "Socket.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>

int Socket::getFd(void) const { return (fd); }
int Socket::getPort(void) const { return (port); }
std::string Socket::getHost(void) const { return (host); }
bool Socket::isValid(void) const { return (fd >= 0); }

Socket::Socket(const std::string &host, int port)
    : fd(-1), port(port), host(host)
{
}

Socket::~Socket() { close(); }

bool Socket::create(void)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        std::cerr << "Failed to create socket for " << host << ":" << port << std::endl;
        return (false);
    }
    if (!setReuseAddr())
    {
        ::close(fd);
        fd = -1;
        return (false);
    }

    return (true);
}

bool Socket::setReuseAddr(void)
{
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "Failed to set SO_REUSEADDR for " << host << ":" << port << std::endl;
        return (false);
    }
    return (true);
}

bool Socket::bindAndListen(void)
{
    if (fd < 0)
    {
        std::cerr << "Cannot bind: socket not created for " << host << ":" << port << std::endl;
        return (false);
    }
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind failed on " << host << ":" << port << std::endl;
        return (false);
    }
    if (listen(fd, 128) < 0)
    {
        std::cerr << "Listen failed on " << host << ":" << port << std::endl;
        return (false);
    }
    return (true);
}

int Socket::acceptConnection(void)
{
    if (fd < 0)
        return (-1);
    int client_fd = accept(fd, NULL, NULL);
    if (client_fd < 0)
    {
        std::cerr << "Accept failed on " << host << ":" << port << std::endl;
        return (-1);
    }
    return (client_fd);
}

void Socket::close(void)
{
    if (fd >= 0)
    {
        ::close(fd);
        fd = -1;
    }
}

bool Socket::setNonBlocking(void)
{
    if (fd < 0)
        return (false);
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return (false);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return (false);
    return (true);
}
