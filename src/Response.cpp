#include "Response.hpp"
#include <iostream>
#include <sstream>

Response::e_status Response::getStatus() const { return (status); }
void Response::setStatus(Response::e_status status) { this->status = status; }

Response::Response(const e_version &version, const e_status &status, const HashMap &headers, const std::string &body)
	: Http(version, headers, body), status(status) {}

static std::string getStatusMessage(Response::e_status status)
{
	switch (status)
	{
	case Response::OK:
		return "OK";
	case Response::CREATED:
		return "Created";
	case Response::NO_CONTENT:
		return "No Content";
	case Response::BAD_REQUEST:
		return "Bad Request";
	case Response::UNAUTHORIZED:
		return "Unauthorized";
	case Response::FORBIDDEN:
		return "Forbidden";
	case Response::NOT_FOUND:
		return "Not Found";
	case Response::METHOD_NOT_ALLOWED:
		return "Method Not Allowed";
	case Response::INTERNAL_SERVER_ERROR:
		return "Internal Server Error";
	case Response::NOT_IMPLEMENTED:
		return "Not Implemented";
	case Response::BAD_GATEWAY:
		return "Bad Gateway";
	case Response::SERVICE_UNAVAILABLE:
		return "Service Unavailable";
	default:
		return "Unknown Status";
	}
}

static std::string versionToString(Http::e_version version)
{
	switch (version)
	{
	case Http::HTTP_1_0:
		return "HTTP/1.0";
	case Http::HTTP_1_1:
		return "HTTP/1.1";
	case Http::HTTP_2_0:
		return "HTTP/2.0";
	default:
		return "unknown";
	}
}

static std::string getStatusNumber(Response::e_status status)
{
	std::ostringstream oss;
	oss << static_cast<int>(status);
	return oss.str();
}

std::string Response::stringify() const
{
	std::string str;
	str += versionToString(getVersion()) + " " + getStatusNumber(getStatus()) + " " + getStatusMessage(getStatus()) + "\r\n";
	str += getHeaders().headerify() + "\r\n\r\n";
	str += getBody();
	return (str);
}
