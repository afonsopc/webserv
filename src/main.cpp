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

int main(int argc, char **argv)
{
	if (argc < 2)
		return (std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl, 1);
	std::ifstream file(argv[1]);
	std::ostringstream buffer;
	buffer << file.rdbuf();
	HashMap config(buffer.str());
	WebServ webserv(config);
	webserv.loop();
}
