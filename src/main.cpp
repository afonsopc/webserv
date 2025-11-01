#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>

#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

#include "HashMap.hpp"
#include "Server.hpp"
#include "WebServ.hpp"
#include "Http.hpp"

int *argc_singleton(void)
{
	static int argc;
	return (&argc);
}

char ***argv_singleton(void)
{
	static char **argv;
	return (&argv);
}

char ***envp_singleton(void)
{
	static char **envp;
	return (&envp);
}

WebServ **webserv_singleton(void)
{
	static WebServ *webserv = NULL;
	return (&webserv);
}

int main(int argc, char **argv, char **envp)
{
	*argc_singleton() = argc;
	*argv_singleton() = argv;
	*envp_singleton() = envp;

	std::string config_file;
	if (argc >= 2)
		config_file = argv[1];
	else
	{
		const char *defaults[] = {"./server.json", "./webserv.conf", "./config.json", NULL};
		for (int i = 0; defaults[i] != NULL; ++i)
		{
			std::ifstream test(defaults[i]);
			if (test.good())
			{
				config_file = defaults[i];
				std::cout << "Using default configuration file: " << config_file << std::endl;
				break;
			}
		}
		if (config_file.empty())
		{
			std::cerr << "Usage: " << argv[0] << " [config_file]" << std::endl;
			std::cerr << "No configuration file specified and no default found." << std::endl;
			std::cerr << "Tried: ./server.json, ./webserv.conf, ./config.json" << std::endl;
			return (1);
		}
	}

	std::ifstream file(config_file.c_str());
	if (!file.good())
	{
		std::cerr << "Error: Cannot open configuration file: " << config_file << std::endl;
		return (1);
	}

	std::ostringstream buffer;
	buffer << file.rdbuf();
	try
	{
		HashMap config(buffer.str());
		WebServ webserv(config);
		*webserv_singleton() = &webserv;
		webserv.loop();
	}
	catch (const std::exception &e)
	{
		std::cerr << "\nError: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}
