#ifndef ROUTE_HPP
#define ROUTE_HPP

#include <string>
#include "Response.hpp"
#include "Request.hpp"
#include <dirent.h>
#include <map>

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
	std::map<std::string, std::string> getExtensions(void) const;

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
	std::string getCgiExecPath(const std::string &filePath) const;
	Response *serveCgiResponse(Request &req, const std::string &filePath, const std::string &execPath);
	Response *parseCgiOutput(const std::string &cgiOutput);
	Response *serveFileResponse(const std::string &filePath);
	Response *checkIndexFiles(Request &req, const std::string &matchedPath);
	Response *handleDirectoryListing(const std::string &matchedPath, const std::string &requestPath);

	std::string path;
	std::string redirect;
	std::vector<std::string> index;
	std::string directory;
	bool directory_listing;
	bool cgi;
	std::vector<std::string> methods;
	std::map<std::string, std::string> extensions;
};

#endif
