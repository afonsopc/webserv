#include "Config.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>

// put parser here
Config parseConfig(const std::string &filePath)
{
	std::ifstream file(filePath);
	if (!file.is_open())
		throw std::runtime_error("Could not open config file: " + filePath);

	Config config;
	// put parser here
	config.port = 8080;	 // parse from parser
	config.backlog = 10; // heretoo

	std::string line;
	while (std::getline(file, line))
	{
		// do the actual parsing here
	}

	return config;
}