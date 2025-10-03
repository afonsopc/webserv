#ifndef ROUTE_HPP
#define ROUTE_HPP

#include <string>
#include "Response.hpp"
#include "Request.hpp"
#include <dirent.h>

class Route
{
public:
	Route(const HashMap &config);
	bool matches(const Request &req) const;
	std::string getPath(void) const;
	std::string getRedirect(void) const;
	std::vector<std::string> getIndex(void) const;
	std::string getDirectory(void) const;
	bool getDirectoryListing(void) const;
	std::vector<std::string> getMethods(void) const;
	bool getCgi(void) const;

	Response *handleRequest(Request &req);

private:
	void load_config(const HashMap &config);
	void assert_config(void);
	std::string getMatchedPath(const Request &req) const;
	Response *redirectResponse(void) const;
	Response *directoryResponse(Request &req);
	Response *directoryListingResponse(std::string dirPath, std::string requestPath, DIR *dir);
	Response *notFoundResponse(void) const;
	Response *fileResponse(Request &req, const std::string &filePath);

	std::string path;
	std::string redirect;
	std::vector<std::string> index;
	std::string directory;
	bool directory_listing;
	bool cgi;
	std::vector<std::string> methods;
};

#endif
