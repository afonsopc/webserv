#include "Server.hpp"
#include "HashMap.hpp"
#include "Socket.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

Server::~Server() { delete (socket); }
int Server::getPort(void) const { return (socket->getPort()); }
std::string Server::getHost(void) const { return (socket->getHost()); }
Socket &Server::getSocket(void) { return (*socket); }
const Socket &Server::getSocket(void) const { return (*socket); }
const std::map<std::string, std::string> &Server::getExtensions(void) const { return extensions; }
size_t Server::getMaxBodySize(void) const { return max_body_size; }

std::string Server::getErrorPage(int status_code) const
{
	std::map<int, std::string>::const_iterator it = error_pages.find(status_code);
	if (it != error_pages.end())
	{
		std::ifstream file(it->second.c_str());
		if (file.is_open())
		{
			std::ostringstream oss;
			oss << file.rdbuf();
			return oss.str();
		}
	}

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

Server::Server(const HashMap &config)
	: socket(new Socket(config.get("host").asString(), config.get("port").asInt())),
	  max_body_size(1048576)
{
	if (config.has("max_body_size"))
		max_body_size = static_cast<size_t>(config.get("max_body_size").asInt());

	if (config.has("error_pages"))
	{
		std::vector<HashMapValue> errorPagesArray = config.get("error_pages").asArray();
		for (std::vector<HashMapValue>::const_iterator it = errorPagesArray.begin(); it != errorPagesArray.end(); ++it)
		{
			HashMap errorMap = it->asHashMap();
			int code = errorMap.get("code").asInt();
			std::string page = errorMap.get("page").asString();
			error_pages[code] = page;
		}
	}

	if (config.has("extensions"))
	{
		std::vector<HashMapValue> extensionsArray = config.get("extensions").asArray();
		for (std::vector<HashMapValue>::const_iterator it = extensionsArray.begin(); it != extensionsArray.end(); ++it)
		{
			HashMap extMap = it->asHashMap();
			std::string ext = extMap.get("ext").asString();
			std::string exec = extMap.get("exec").asString();
			extensions[ext] = exec;
		}
	}
	std::vector<HashMapValue> routesArray = config.get("routes").asArray();
	for (std::vector<HashMapValue>::const_iterator it = routesArray.begin(); it != routesArray.end(); ++it)
	{
		std::cout << "  Loading route #" << (routes.size() + 1) << " for path: " << it->asHashMap().get("path").asString() << std::endl;
		routes.push_back(Route(it->asHashMap(), extensions));
	}
}

bool Server::initialize(void)
{
	if (!socket->create())
	{
		std::cerr << "Failed to create socket for server on " << socket->getHost() << ":" << socket->getPort() << std::endl;
		return (false);
	}
	if (!socket->bindAndListen())
	{
		std::cerr << "Failed to bind/listen for server on " << socket->getHost() << ":" << socket->getPort() << std::endl;
		socket->close();
		return (false);
	}
	if (!socket->setNonBlocking())
	{
		std::cerr << "Failed to set non-blocking for server on " << socket->getHost() << ":" << socket->getPort() << std::endl;
		socket->close();
		return (false);
	}
	return (true);
}

Response *Server::handleRequest(Request &req)
{
	if (req.getVersion() == Http::HTTP_1_1)
	{
		if (!req.getHeaders().get("Host").isString())
		{
			HashMap headers = HashMap();
			headers.set("Content-Type", "text/html");
			return (new Response(Http::HTTP_1_1, 400, headers, getErrorPage(400)));
		}
		std::string server_host = getHost();
		if (server_host != "0.0.0.0" && server_host != "localhost")
		{
			std::string host_header = req.getHeaders().get("Host").asString();
			size_t colon_pos = host_header.find(':');
			std::string request_host = host_header.substr(0, colon_pos);
			if (request_host != server_host)
			{
				HashMap headers = HashMap();
				headers.set("Content-Type", "text/html");
				return (new Response(Http::HTTP_1_1, 400, headers, getErrorPage(400)));
			}
		}
	}
	for (size_t i = 0; i < routes.size(); ++i)
		if (routes[i].matches(req))
			return (routes[i].handleRequest(req));
	HashMap headers = HashMap();
	headers.set("Content-Type", "text/html");
	return (new Response(Http::HTTP_1_1, 404, headers, getErrorPage(404)));
}
