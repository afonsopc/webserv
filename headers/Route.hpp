#ifndef ROUTE_HPP
#define ROUTE_HPP

#include <string>
#include "Response.hpp"
#include "Request.hpp"

class Route
{
public:
	Route(const HashMap &config);
	bool matches(const Request &req) const;

	Response *handleRequest(Request &req);

private:
	void load_config(const HashMap &config);
	void assert_config(void);
	std::string getMatchedPath(const Request &req) const;
	Response *redirectResponse(void) const;

	std::string path;
	std::string redirect;
	std::vector<std::string> index;
	std::string directory;
	bool directory_listing;
	std::vector<std::string> methods;
};

#endif
