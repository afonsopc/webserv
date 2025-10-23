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
#include <cctype>

std::string Route::getPath(void) const { return path; }
std::string Route::getRedirect(void) const { return redirect; }
std::vector<std::string> Route::getIndex(void) const { return index; }
std::string Route::getDirectory(void) const { return directory; }
bool Route::getDirectoryListing(void) const { return directory_listing; }
std::vector<std::string> Route::getMethods(void) const { return methods; }
bool Route::getCgi(void) const { return cgi; }
std::map<std::string, std::string> Route::getExtensions(void) const { return extensions; }

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
	if (config.has("extensions"))
	{
		std::vector<HashMapValue> extArray = config.get("extensions").asArray();
		for (size_t i = 0; i < extArray.size(); ++i)
		{
			HashMap extObj = extArray[i].asHashMap();
			std::string ext = extObj.get("ext").asString();
			std::string exec = extObj.get("exec").asString();
			extensions[ext] = exec;
		}
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
	int status = 301;
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
	return (new Response(Http::HTTP_1_1, 418, headers, body.str()));
}

std::string execCgi(Request &req, const std::string &filePath, const std::string &execPath)
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

		// Build CGI environment variables
		std::vector<std::string> env_vars;

		// Add REQUEST_METHOD
		const char *method_names[] = {"GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH", "UNKNOWN"};
		env_vars.push_back(std::string("REQUEST_METHOD=") + method_names[req.getMethod()]);

		// Parse query string from path
		std::string path = req.getPath();
		size_t query_pos = path.find('?');
		std::string query_string = (query_pos != std::string::npos) ? path.substr(query_pos + 1) : "";
		env_vars.push_back(std::string("QUERY_STRING=") + query_string);

		// Add CONTENT_LENGTH
		std::ostringstream oss;
		oss << req.getBody().size();
		env_vars.push_back(std::string("CONTENT_LENGTH=") + oss.str());

		// Add CONTENT_TYPE if present
		HashMap headers = req.getHeaders();
		if (headers.has("Content-Type"))
			env_vars.push_back(std::string("CONTENT_TYPE=") + headers.get("Content-Type").asString());

		// Add HTTP_* headers
		std::vector<std::string> header_keys = headers.keys();
		for (size_t i = 0; i < header_keys.size(); ++i)
		{
			std::string key = header_keys[i];
			std::string env_key = "HTTP_" + key;
			for (size_t j = 0; j < env_key.size(); ++j)
				if (env_key[j] == '-')
					env_key[j] = '_';
			for (size_t j = 0; j < env_key.size(); ++j)
				if (islower(env_key[j]))
					env_key[j] = toupper(env_key[j]);
			env_vars.push_back(env_key + "=" + headers.get(key).asString());
		}

		// Add original environment variables
		char **orig_env = *envp_singleton();
		for (int i = 0; orig_env[i] != NULL; ++i)
			env_vars.push_back(orig_env[i]);

		// Convert to char **
		char **cgi_envp = new char *[env_vars.size() + 1];
		for (size_t i = 0; i < env_vars.size(); ++i)
			cgi_envp[i] = const_cast<char *>(env_vars[i].c_str());
		cgi_envp[env_vars.size()] = NULL;

		std::string rawRequest = req.getRaw();
		if (execPath.empty())
		{
			char *args[] = {const_cast<char *>(filePath.c_str()), const_cast<char *>(rawRequest.c_str()), NULL};
			execve(filePath.c_str(), args, cgi_envp);
		}
		else
		{
			char *args[] = {const_cast<char *>(execPath.c_str()), const_cast<char *>(filePath.c_str()), const_cast<char *>(rawRequest.c_str()), NULL};
			execve(execPath.c_str(), args, cgi_envp);
		}
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

static bool isRegularFile(const char *path)
{
	struct stat st;

	if (stat(path, &st) != 0)
		return (false);
	return (S_ISREG(st.st_mode));
}

Response *Route::fileResponse(Request &req, const std::string &filePath)
{
	if (!isRegularFile(filePath.c_str()))
		return (notFoundResponse());

	std::string execPath;
	bool isCgi = cgi;
	if (isCgi)
	{
		size_t dot = filePath.find_last_of('.');
		if (dot != std::string::npos)
		{
			std::string ext = filePath.substr(dot + 1);
			if (extensions.count(ext))
			{
				execPath = extensions[ext];
			}
			else
			{
				isCgi = false;
			}
		}
		else
		{
			isCgi = false;
		}
	}

	if (isCgi)
	{
		std::string cgiOutput = execCgi(req, filePath, execPath);
		std::istringstream iss(cgiOutput);
		std::string statusLine;
		std::getline(iss, statusLine);
		if (!statusLine.empty() && statusLine[statusLine.size() - 1] == '\r')
			statusLine.erase(statusLine.size() - 1);
		int status = 200;
		HashMap headers = HashMap();
		if (!statusLine.empty() && isdigit(statusLine[0]))
			status = std::atoi(statusLine.c_str());
		else if (!statusLine.empty())
		{
			// parse as header
			size_t colonPos = statusLine.find(':');
			if (colonPos != std::string::npos)
			{
				std::string key = statusLine.substr(0, colonPos);
				std::string value = statusLine.substr(colonPos + 1);
				key.erase(0, key.find_first_not_of(" \t"));
				key.erase(key.find_last_not_of(" \t") + 1);
				value.erase(0, value.find_first_not_of(" \t"));
				value.erase(value.find_last_not_of(" \t") + 1);
				headers.set(key, value);
			}
		}
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
		std::ostringstream oss;
		oss << iss.rdbuf();
		std::string body = oss.str();
		Http::e_version version = Http::HTTP_1_1;
		return (new Response(version, status, headers, body));
	}
	else
	{
		// serve as file
		HashMap headers = HashMap();
		std::ifstream file(filePath.c_str());
		std::ostringstream oss;
		oss << file.rdbuf();
		std::string body = oss.str();
		Http::e_version version = Http::HTTP_1_1;
		return (new Response(version, 200, headers, body));
	}
}

Response *Route::notFoundResponse(void) const { return (new Response(Http::HTTP_1_1, 404, HashMap(), "404 Not Found\n")); }

Response *Route::directoryResponse(Request &req)
{
	std::string matchedPath = directory + "/" + getMatchedPath(req);
	std::cout << matchedPath << std::endl;
	if (isRegularFile(matchedPath.c_str()))
		return (fileResponse(req, matchedPath));
	if (!index.empty())
	{
		for (size_t i = 0; i < index.size(); ++i)
		{
			std::cout << "Checking index: " << index[i] << std::endl;
			std::string indexPath = matchedPath;
			if (indexPath.empty() || indexPath[indexPath.size() - 1] != '/')
				indexPath += "/";
			indexPath += index[i];
			if (isRegularFile(indexPath.c_str()))
				return (fileResponse(req, indexPath));
		}
		if (cgi && !index.empty())
		{
			std::string fallbackPath = directory + "/" + index[0];
			if (isRegularFile(fallbackPath.c_str()))
				return (fileResponse(req, fallbackPath));
		}
		return (notFoundResponse());
	}
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
