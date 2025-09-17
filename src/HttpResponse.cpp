#include "HttpResponse.hpp"
#include <sstream>

HttpResponse::HttpResponse() : version("HTTP/1.1"), status_code(200), status_text("OK") {}

std::string HttpResponse::toString() const
{
	std::ostringstream oss;
	oss << version << " " << status_code << " " << status_text << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		oss << it->first << ": " << it->second << "\r\n";

	oss << "\r\n"
		<< body;
	return oss.str();
}

void HttpResponse::setHeader(const std::string &key, const std::string &value)
{
	headers[key] = value;
}

const std::map<std::string, std::string> &HttpResponse::getHeaders() const
{
	return headers;
}

void HttpResponse::setStatus(int code, const std::string &text)
{
	status_code = code;
	status_text = text;
}

void HttpResponse::setBody(const std::string &content)
{
	body = content;
	setHeader("Content-Length", std::to_string(body.length()));
}

const std::string &HttpResponse::getVersion() const
{
	return version;
}

int HttpResponse::getStatusCode() const
{
	return status_code;
}

const std::string &HttpResponse::getStatusText() const
{
	return status_text;
}

const std::string &HttpResponse::getBody() const
{
	return body;
}