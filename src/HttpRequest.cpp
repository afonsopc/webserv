#include "HttpRequest.hpp"
#include <sstream>
#include <iostream>

HttpRequest::HttpRequest(const std::string &request_str)
{
	parseRequest(request_str);
}

std::string HttpRequest::toString() const
{
	std::ostringstream oss;
	oss << method << " " << path << " " << version << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
		oss << it->first << ": " << it->second << "\r\n";

	return oss.str();
}

void HttpRequest::setHeader(const std::string &key, const std::string &value)
{
	headers[key] = value;
}

const std::map<std::string, std::string> &HttpRequest::getHeaders() const
{
	return headers;
}

void HttpRequest::parseRequest(const std::string &request_str)
{
	std::istringstream iss(request_str);
	std::string line;

	if (std::getline(iss, line))
	{
		std::istringstream line_stream(line);
		line_stream >> method >> path >> version;
	}

	while (std::getline(iss, line) && line != "\r")
	{
		size_t colon_pos = line.find(':');
		if (colon_pos != std::string::npos)
		{
			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 2);
			if (!value.empty() && value.back() == '\r')
				value.pop_back();
			headers[key] = value;
		}
	}
}

const std::string &HttpRequest::getMethod() const
{
	return method;
}

const std::string &HttpRequest::getPath() const
{
	return path;
}

const std::string &HttpRequest::getVersion() const
{
	return version;
}