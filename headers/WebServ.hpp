#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <vector>
#include "Server.hpp"

class WebServ
{
public:
    WebServ(const std::vector<Server> &servers);

private:
    std::vector<Server> servers;
};

#endif
