#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

struct Config
{
	int port;
	int backlog;
};

Config parseConfig(const std::string &filePath);

#endif