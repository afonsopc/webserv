#include "Request.hpp"
#include <iostream>

Request::e_method Request::getMethod() const { return (method); }
void Request::setMethod(Request::e_method method) { this->method = method; }
std::string Request::getPath() const { return (path); }
void Request::setPath(const std::string &path) { this->path = path; }

static Http::e_version parse_http_version(const std::string &raw)
{
	size_t pos = raw.find("\r\n");
	if (pos == std::string::npos)
		return Http::UNKNOWN_VERSION;
	std::string first_line = raw.substr(0, pos);
	size_t last_space = first_line.rfind(' ');
	if (last_space == std::string::npos)
		return Http::UNKNOWN_VERSION;
	std::string version_str = first_line.substr(last_space + 1);
	if (version_str == "HTTP/1.0")
		return Http::HTTP_1_0;
	else if (version_str == "HTTP/1.1")
		return Http::HTTP_1_1;
	else if (version_str == "HTTP/2.0")
		return Http::HTTP_2_0;
	return Http::UNKNOWN_VERSION;
}

static HashMap parse_http_headers(const std::string &raw)
{
	HashMap headers;
	size_t pos = raw.find("\r\n");
	if (pos == std::string::npos)
		return (headers);
	size_t start = pos + 2;
	while (true)
	{
		size_t end = raw.find("\r\n", start);
		if (end == std::string::npos || end == start)
			break;
		std::string line = raw.substr(start, end - start);
		size_t colon = line.find(':');
		if (colon != std::string::npos)
		{
			std::string key = line.substr(0, colon);
			std::string value = line.substr(colon + 1);
			key.erase(0, key.find_first_not_of(" \t"));
			key.erase(key.find_last_not_of(" \t") + 1);
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);
			headers.set(key, value);
		}
		start = end + 2;
	}
	return (headers);
}

static std::string parse_http_body(const std::string &raw)
{
	size_t pos = raw.find("\r\n\r\n");
	if (pos == std::string::npos)
		return ("");
	return (raw.substr(pos + 4));
}

static std::string parse_http_path(const std::string &raw)
{
	size_t pos = raw.find("\r\n");
	if (pos == std::string::npos)
		return ("");
	std::string first_line = raw.substr(0, pos);
	size_t first_space = first_line.find(' ');
	if (first_space == std::string::npos)
		return ("");
	size_t second_space = first_line.find(' ', first_space + 1);
	if (second_space == std::string::npos)
		return ("");
	return (first_line.substr(first_space + 1, second_space - first_space - 1));
}

static Request::e_method parse_http_method(const std::string &raw)
{
	size_t pos = raw.find("\r\n");
	if (pos == std::string::npos)
		return (Request::UNKNOWN);
	std::string first_line = raw.substr(0, pos);
	size_t first_space = first_line.find(' ');
	if (first_space == std::string::npos)
		return (Request::UNKNOWN);
	std::string method_str = first_line.substr(0, first_space);
	if (method_str == "GET")
		return (Request::GET);
	else if (method_str == "POST")
		return (Request::POST);
	else if (method_str == "PUT")
		return (Request::PUT);
	else if (method_str == "DELETE")
		return (Request::DELETE);
	else if (method_str == "HEAD")
		return (Request::HEAD);
	else if (method_str == "OPTIONS")
		return (Request::OPTIONS);
	else if (method_str == "PATCH")
		return (Request::PATCH);
	return (Request::UNKNOWN);
}

Request::Request(const std::string &raw) : Http(parse_http_version(raw), parse_http_headers(raw), parse_http_body(raw)),
										   method(parse_http_method(raw)),
										   path(parse_http_path(raw))
{
}

std::ostream &operator<<(std::ostream &os, const Request &request)
{
	const char *method_names[] = {"GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS", "PATCH", "UNKNOWN"};
	const char *version_names[] = {"HTTP/1.0", "HTTP/1.1", "HTTP/2.0", "UNKNOWN"};
	os << method_names[request.method] << " " << request.path << " " << version_names[request.getVersion()] << "\r\n";
	HashMap headers = request.getHeaders();
	std::vector<std::string> keys = headers.keys();
	for (size_t i = 0; i < keys.size(); i++)
		os << keys[i] << ": " << headers.get(keys[i]).asString() << "\r\n";
	os << "\r\n"
	   << request.getBody();
	return (os);
}