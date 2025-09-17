#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "Http.hpp"
#include <string>
#include <map>

class HttpResponse : public Http
{
private:
	std::string version;
	int status_code;
	std::string status_text;
	std::map<std::string, std::string> headers;
	std::string body;

public:
	HttpResponse();
	~HttpResponse() {}

	std::string toString() const;
	void setHeader(const std::string &key, const std::string &value);
	const std::map<std::string, std::string> &getHeaders() const;

	void setStatus(int code, const std::string &text);
	void setBody(const std::string &content);
	const std::string &getVersion() const;
	int getStatusCode() const;
	const std::string &getStatusText() const;
	const std::string &getBody() const;
};

#endif