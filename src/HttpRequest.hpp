#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "Http.hpp"
#include <string>
#include <map>

class HttpRequest : public Http
{
private:
	std::string method;
	std::string path;
	std::string version;
	std::map<std::string, std::string> headers;

public:
	HttpRequest(const std::string &request_str);
	~HttpRequest() {}

	std::string toString() const;
	void setHeader(const std::string &key, const std::string &value);
	const std::map<std::string, std::string> &getHeaders() const;

	void parseRequest(const std::string &request_str);
	const std::string &getMethod() const;
	const std::string &getPath() const;
	const std::string &getVersion() const;
};

#endif