#include "ErrorPages.hpp"
#include <fstream>
#include <sstream>

std::string ErrorPages::getDefaultErrorPage(int status_code)
{
	std::ostringstream default_path;
	default_path << "www/errors/" << status_code << ".html";
	std::ifstream default_file(default_path.str().c_str());
	if (default_file.is_open())
	{
		std::ostringstream oss;
		oss << default_file.rdbuf();
		return oss.str();
	}

	std::ostringstream oss;
	oss << status_code << " Error\n";
	return oss.str();
}
