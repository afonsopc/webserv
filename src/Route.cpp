#include "WebServ.hpp"
#include "Route.hpp"
#include "HashMap.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/wait.h>

std::string Route::getPath(void) const { return path; }
std::string Route::getRedirect(void) const { return redirect; }
std::vector<std::string> Route::getIndex(void) const { return index; }
std::string Route::getDirectory(void) const { return directory; }
bool Route::getDirectoryListing(void) const { return directory_listing; }
std::vector<std::string> Route::getMethods(void) const { return methods; }
bool Route::getCgi(void) const { return cgi; }

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
	cgi = config.has("cgi") && config.get("cgi").asBool();
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

Response *Route::directoryListingResponse(std::string dirPath, std::string requestPath, DIR *dir)
{
	std::stringstream body;

	body << "<html><body><h1>Directory listing for " << dirPath << "</h1><ul>";
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		std::string href = requestPath;
		if (href.empty() || href[href.size() - 1] != '/')
			href += "/";
		href += name;
		body << "<li><a href=\"" << href << "\">" << name << "</a></li>";
	}
	body << "</ul></body></html>";
	closedir(dir);
	HashMap headers = HashMap();
	headers.set("Content-Type", "text/html");
	return (new Response(Http::HTTP_1_1, Response::IM_A_TEAPOT, headers, body.str()));
}

std::string execCgi(Request &req, const std::string &filePath)
{
	int pipefd[2];
	if (pipe(pipefd) == -1)
		return ("CGI Error: Failed to create pipe\n");
	pid_t pid = fork();
	if (pid == -1)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		return ("CGI Error: Failed to fork\n");
	}
	if (pid == 0)
	{
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		std::string rawRequest = req.getRaw();
		char *args[] = {const_cast<char *>(filePath.c_str()), const_cast<char *>(rawRequest.c_str()), NULL};
		execve(filePath.c_str(), args, *envp_singleton());
		std::cerr << "CGI Error: Failed to execute " << filePath << std::endl;
		exit(1);
	}
	else
	{
		close(pipefd[1]);
		std::ostringstream output;
		char buffer[4096];
		ssize_t bytesRead;
		while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
			output.write(buffer, bytesRead);
		close(pipefd[0]);
		int status;
		waitpid(pid, &status, 0);
		return (output.str());
	}
}

Response *Route::fileResponse(Request &req, const std::string &filePath)
{
	HashMap headers = HashMap();
	std::ifstream file(filePath.c_str());
	if (!file.is_open())
		return (notFoundResponse());
	std::ostringstream oss;
	std::string firstLine;
	oss << file.rdbuf();
	std::istringstream iss(oss.str());
	std::getline(iss, firstLine);
	if (cgi && firstLine.size() >= 2 && firstLine[0] == '#' && firstLine[1] == '!')
	{
		oss.str("");
		std::string cgiOutput = execCgi(req, filePath);
		std::istringstream iss(cgiOutput);
		while (true)
		{
			std::string header;
			getline(iss, header);
			if (header.empty() || header == "\r")
				break;
			size_t colonPos = header.find(':');
			if (colonPos != std::string::npos)
			{
				std::string key = header.substr(0, colonPos);
				std::string value = header.substr(colonPos + 1);
				headers.set(key, value);
			}
		}
		oss << iss.rdbuf();
	}
	std::string body = oss.str();
	Response::e_status status = Response::OK;
	Http::e_version version = Http::HTTP_1_1;
	return (new Response(version, status, headers, body));
}

Response *Route::notFoundResponse(void) const
{
	Response::e_status status = Response::NOT_FOUND;
	std::string body = "404 Not Found\n";
	Http::e_version version = Http::HTTP_1_1;
	HashMap headers = HashMap();
	return (new Response(version, status, headers, body));
}

static bool isRegularFile(const char *path)
{
	struct stat st;

	if (stat(path, &st) != 0)
		return (false);
	return (S_ISREG(st.st_mode));
}

Response *Route::directoryResponse(Request &req)
{
	std::string matchedPath = directory + "/" + getMatchedPath(req);
	std::cout << matchedPath << std::endl;
	if (isRegularFile(matchedPath.c_str()))
		return (fileResponse(req, matchedPath));
	if (directory_listing)
	{
		DIR *dir = opendir(matchedPath.c_str());
		if (dir)
			return (directoryListingResponse(matchedPath, req.getPath(), dir));
		return (notFoundResponse());
	}
	return (NULL);
}

Response *Route::handleRequest(Request &req)
{
	if (!redirect.empty())
		return (redirectResponse());
	if (!directory.empty())
		return (directoryResponse(req));
	return (NULL);
}
