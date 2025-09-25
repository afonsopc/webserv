#ifndef ROUTE_HPP
#define ROUTE_HPP

#include <string>
#include "Response.hpp"
#include "Request.hpp"

class Route
{
public:
	Route(const HashMap &config);

	Response *handleRequest(Request &req);

private:
	std::string path;
	std::string redirect;
	std::vector<std::string> index;
	std::string directory;
	bool directory_listing;
	std::vector<std::string> methods;
};

#endif
