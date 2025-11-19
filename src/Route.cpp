#include "WebServ.hpp"
#include "Route.hpp"
#include "HashMap.hpp"
#include "ErrorPages.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

std::string Route::getPath(void) const { return path; }
std::string Route::getRedirect(void) const { return redirect; }
std::vector<std::string> Route::getIndex(void) const { return index; }
std::string Route::getDirectory(void) const { return directory; }
std::string Route::getUploadDir(void) const { return upload_dir; }
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
	if (config.has("upload_dir"))
		upload_dir = config.get("upload_dir").asString();
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

Route::Route(const HashMap &config, const std::map<std::string, std::string> &exts)
	: extensions(exts)
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
	return (new Response(Http::HTTP_1_1, 200, headers, body.str()));
}

Response *Route::fileResponse(Request &req, const std::string &filePath)
{
	(void)req;
	HashMap headers = HashMap();
	size_t dotPos = filePath.find_last_of('.');
	std::string ext = (dotPos != std::string::npos) ? filePath.substr(dotPos + 1) : "";
	bool isCgiFile = cgi && extensions.count(ext);
	if (isCgiFile)
	{
		std::string interpreter = extensions.at(ext);
		headers.set("X-CGI-Interpreter", interpreter);
		headers.set("X-CGI-Script", filePath);
		return (new Response(Http::HTTP_1_1, -1, headers, ""));
	}
	else
	{
		std::ifstream file(filePath.c_str());
		if (!file.is_open())
			return (notFoundResponse());
		std::ostringstream oss;
		oss << file.rdbuf();

		std::string content_type = "application/octet-stream";
		if (ext == "html" || ext == "htm")
			content_type = "text/html";
		else if (ext == "css")
			content_type = "text/css";
		else if (ext == "js")
			content_type = "application/javascript";
		else if (ext == "json")
			content_type = "application/json";
		else if (ext == "xml")
			content_type = "application/xml";
		else if (ext == "txt")
			content_type = "text/plain";
		else if (ext == "jpg" || ext == "jpeg")
			content_type = "image/jpeg";
		else if (ext == "png")
			content_type = "image/png";
		else if (ext == "gif")
			content_type = "image/gif";
		else if (ext == "svg")
			content_type = "image/svg+xml";
		else if (ext == "ico")
			content_type = "image/x-icon";
		else if (ext == "pdf")
			content_type = "application/pdf";

		headers.set("Content-Type", content_type);
		return (new Response(Http::HTTP_1_1, 200, headers, oss.str()));
	}
}

Response *Route::notFoundResponse(void) const
{
	HashMap headers = HashMap();
	headers.set("Content-Type", "text/html");
	return (new Response(Http::HTTP_1_1, 404, headers, ErrorPages::getDefaultErrorPage(404)));
}

Response *Route::handleFileUpload(Request &req)
{
	if (upload_dir.empty())
	{
		HashMap headers = HashMap();
		headers.set("Content-Type", "text/html");
		return (new Response(Http::HTTP_1_1, 403, headers, ErrorPages::getDefaultErrorPage(403)));
	}

	std::string content_type = req.getHeaders().get("Content-Type").isString()
								   ? req.getHeaders().get("Content-Type").asString()
								   : "";

	std::string body = req.getBody();
	if (body.empty())
	{
		HashMap headers = HashMap();
		headers.set("Content-Type", "text/html");
		return (new Response(Http::HTTP_1_1, 400, headers, ErrorPages::getDefaultErrorPage(400)));
	}

	std::string filename = "upload_";
	std::ostringstream oss;
	oss << time(NULL);
	filename += oss.str();

	std::string path_str = req.getPath();
	size_t last_slash = path_str.find_last_of('/');
	if (last_slash != std::string::npos && last_slash + 1 < path_str.length())
		filename = path_str.substr(last_slash + 1);

	std::string filepath = upload_dir + "/" + filename;
	std::ofstream outfile(filepath.c_str(), std::ios::binary);
	if (!outfile.is_open())
	{
		HashMap headers = HashMap();
		headers.set("Content-Type", "text/html");
		return (new Response(Http::HTTP_1_1, 500, headers, ErrorPages::getDefaultErrorPage(500)));
	}

	outfile.write(body.c_str(), body.length());
	outfile.close();

	HashMap headers = HashMap();
	headers.set("Content-Type", "text/plain");
	std::string response_body = "File uploaded successfully to: " + filepath + "\n";
	return (new Response(Http::HTTP_1_1, 201, headers, response_body));
}

static bool isRegularFile(const char *path)
{
	struct stat st;

	if (stat(path, &st) != 0)
		return (false);
	return (S_ISREG(st.st_mode));
}

static bool isDirectory(const char *path)
{
	struct stat st;

	if (stat(path, &st) != 0)
		return (false);
	return (S_ISDIR(st.st_mode));
}

Response *Route::directoryResponse(Request &req)
{
	std::string matchedPath = directory + "/" + getMatchedPath(req);

	if (isRegularFile(matchedPath.c_str()))
		return (fileResponse(req, matchedPath));

	if (isDirectory(matchedPath.c_str()))
	{
		if (!index.empty())
		{
			for (size_t i = 0; i < index.size(); ++i)
			{
				std::string indexPath = matchedPath;
				if (indexPath.empty() || indexPath[indexPath.size() - 1] != '/')
					indexPath += "/";
				indexPath += index[i];
				if (isRegularFile(indexPath.c_str()))
					return (fileResponse(req, indexPath));
			}
			if (cgi && !index.empty())
			{
				std::string fallbackPath = matchedPath;
				if (fallbackPath.empty() || fallbackPath[fallbackPath.size() - 1] != '/')
					fallbackPath += "/";
				fallbackPath += index[0];
				return (fileResponse(req, fallbackPath));
			}
		}
		if (directory_listing)
		{
			DIR *dir = opendir(matchedPath.c_str());
			if (dir)
				return (directoryListingResponse(matchedPath, req.getPath(), dir));
		}
	}

	if (cgi && !index.empty())
	{
		std::string cgiPath = directory + "/" + index[0];
		return (fileResponse(req, cgiPath));
	}

	return (notFoundResponse());
}

Response *Route::handleRequest(Request &req)
{
	std::string method_str;
	switch (req.getMethod())
	{
	case Request::GET:
		method_str = "GET";
		break;
	case Request::POST:
		method_str = "POST";
		break;
	case Request::PUT:
		method_str = "PUT";
		break;
	case Request::DELETE:
		method_str = "DELETE";
		break;
	case Request::HEAD:
		method_str = "HEAD";
		break;
	case Request::OPTIONS:
		method_str = "OPTIONS";
		break;
	case Request::PATCH:
		method_str = "PATCH";
		break;
	default:
		method_str = "UNKNOWN";
		break;
	}

	bool method_allowed = false;
	for (size_t i = 0; i < methods.size(); ++i)
	{
		if (methods[i] == method_str)
		{
			method_allowed = true;
			break;
		}
	}

	if (!method_allowed)
	{
		HashMap headers = HashMap();
		std::string allowed_methods;
		for (size_t i = 0; i < methods.size(); ++i)
		{
			if (i > 0)
				allowed_methods += ", ";
			allowed_methods += methods[i];
		}
		headers.set("Allow", allowed_methods);
		headers.set("Content-Type", "text/html");
		return (new Response(Http::HTTP_1_1, 405, headers, ErrorPages::getDefaultErrorPage(405)));
	}

	if (req.getMethod() == Request::POST && !upload_dir.empty())
		return (handleFileUpload(req));

	if (req.getMethod() == Request::DELETE && !directory.empty())
	{
		std::string matchedPath = getMatchedPath(req);
		std::string filepath = directory + "/" + matchedPath;

		if (matchedPath.empty() || matchedPath.find("..") != std::string::npos)
		{
			HashMap headers = HashMap();
			headers.set("Content-Type", "text/html");
			return (new Response(Http::HTTP_1_1, 403, headers, ErrorPages::getDefaultErrorPage(403)));
		}

		if (!isRegularFile(filepath.c_str()))
		{
			HashMap headers = HashMap();
			headers.set("Content-Type", "text/html");
			return (new Response(Http::HTTP_1_1, 404, headers, ErrorPages::getDefaultErrorPage(404)));
		}

		if (remove(filepath.c_str()) == 0)
		{
			HashMap headers = HashMap();
			headers.set("Content-Type", "text/plain");
			std::string body = "File deleted successfully: " + filepath + "\n";
			return (new Response(Http::HTTP_1_1, 200, headers, body));
		}
		else
		{
			HashMap headers = HashMap();
			headers.set("Content-Type", "text/html");
			return (new Response(Http::HTTP_1_1, 500, headers, ErrorPages::getDefaultErrorPage(500)));
		}
	}

	if (!redirect.empty())
		return (redirectResponse());
	if (!directory.empty())
		return (directoryResponse(req));
	return (NULL);
}
