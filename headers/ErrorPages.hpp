#ifndef ERRORPAGES_HPP
#define ERRORPAGES_HPP

#include <string>

class ErrorPages
{
public:
	static std::string getDefaultErrorPage(int status_code);
};

#endif
