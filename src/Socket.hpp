#ifndef SOCKET_INTERFACE_HPP
#define SOCKET_INTERFACE_HPP

#include <string>

class Socket
{
public:
	virtual ~Socket() {}
	virtual int getSocketFd() const = 0;
	virtual void closeSocket() = 0;
	virtual bool isValid() const = 0;
};

#endif