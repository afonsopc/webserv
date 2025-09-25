#include "Route.hpp"
#include "HashMap.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

Route::Route(const HashMap &config)
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
	if (config.has("directory_listing"))
		directory_listing = config.get("directory_listing").asBool();
	if (config.has("methods"))
	{
		std::vector<HashMapValue> methodsArray = config.get("methods").asArray();
		for (size_t i = 0; i < methodsArray.size(); ++i)
			methods.push_back(methodsArray[i].asString());
	}
}

Response *Route::handleRequest(Request &req)
{
	Response::e_status status = Response::OK;
	std::string body = "ola DESCONHECIDO (ANONYMO :O) :)\n";
	if (req.getHeaders().get("Host").isString())
		body = "ola " + req.getHeaders().get("Host").asString() + req.getPath() + " :)\n";
	Http::e_version version = Http::HTTP_1_1;
	HashMap headers = HashMap();
	return (new Response(version, status, headers, body));
}
