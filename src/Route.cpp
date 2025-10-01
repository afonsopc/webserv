#include "Route.hpp"
#include "HashMap.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

void Route::load_config(const HashMap &config)
{
	if (!config.has("path"))
		throw(std::invalid_argument("Route configuration must include a valid 'path' string."));
	path = config.get("path").asString();
	if (config.has("redirect"))
		redirect = config.get("redirect").asString();
	if (config.has("index"))
	{
		std::vector<HashMapValue> indexArray = config.get("index").asArray();
		for (size_t i = 0; i < indexArray.size(); ++i)
			index.push_back(indexArray[i].asString());
	}
	if (config.has("directory"))
		directory = config.get("directory").asString();
	directory_listing = config.has("directory_listing") && config.get("directory_listing").asBool();
	if (config.has("methods"))
	{
		std::vector<HashMapValue> methodsArray = config.get("methods").asArray();
		for (size_t i = 0; i < methodsArray.size(); ++i)
			methods.push_back(methodsArray[i].asString());
	}
}

void Route::assert_config(void)
{
	if (path.empty())
		throw(std::invalid_argument("Route 'path' cannot be empty."));
	if (!redirect.empty() && !directory.empty())
		throw(std::invalid_argument("Route cannot have both 'redirect' and 'directory' set."));
	if (methods.empty())
		throw(std::invalid_argument("Route must have at least one HTTP method in 'methods'."));
	if (!index.empty() && (directory.empty() || directory_listing))
		throw(std::invalid_argument("Route with 'index' must have 'directory' set and 'directory_listing' false."));
	for (size_t i = 0; i < methods.size(); ++i)
		if (methods[i] != "GET" && methods[i] != "POST" && methods[i] != "PUT" &&
			methods[i] != "DELETE" && methods[i] != "HEAD" && methods[i] != "OPTIONS" &&
			methods[i] != "PATCH")
			throw(std::invalid_argument("Route 'methods' contains invalid HTTP method: " + methods[i]));
}

Route::Route(const HashMap &config)
{
	load_config(config);
	assert_config();
}

std::string Route::getMatchedPath(const Request &req) const
{
	std::string reqPath = req.getPath();

	if (path.size() >= 2 && path.substr(path.size() - 2) == "/*")
	{
		std::string prefix = path.substr(0, path.size() - 2);
		if (prefix.empty())
			return (reqPath.substr(1));
		if (reqPath == prefix)
			return ("");
		if (reqPath.size() > prefix.size() &&
			reqPath.substr(0, prefix.size()) == prefix &&
			reqPath[prefix.size()] == '/')
			return (reqPath.substr(prefix.size() + 1));
	}
	else if (reqPath == path)
		return ("");
	return (reqPath);
}

bool Route::matches(const Request &req) const
{
	std::string matchedPath = getMatchedPath(req);
	if (matchedPath != req.getPath())
		return (true);
	return (false);
}

Response *Route::redirectResponse(void) const
{
	if (redirect.empty())
		return (NULL);
	Response::e_status status = Response::MOVED_PERMANENTLY;
	std::string body = "Redirecting to " + redirect + "\n";
	Http::e_version version = Http::HTTP_1_1;
	HashMap headers = HashMap();
	headers.set("Location", redirect);
	return (new Response(version, status, headers, body));
}

Response *Route::handleRequest(Request &req)
{
	if (!redirect.empty())
		return (redirectResponse());
	Response::e_status status = Response::OK;
	std::string body = "ola DESCONHECIDO (ANONYMO :O) :)\n";
	if (req.getHeaders().get("Host").isString())
		body = "ola " + req.getHeaders().get("Host").asString() + req.getPath() + " :)\n";
	Http::e_version version = Http::HTTP_1_1;
	HashMap headers = HashMap();
	return (new Response(version, status, headers, body));
}

// /omelhorsite/storage