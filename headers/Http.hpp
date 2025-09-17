#ifndef HTTP_HPP
#define HTTP_HPP

#include <string>
#include <map>
#include "HashMap.hpp"
#include "Uri.hpp"

enum HttpMethod
{
	GET,
	POST,
	PUT,
	DELETE,
	HEAD,
	OPTIONS,
	PATCH,
	UNKNOWN
};

enum HttpVersion
{
	HTTP_1_0,
	HTTP_1_1,
	HTTP_2_0,
	UNKNOWN_VERSION
};

enum HttpStatus
{
	OK = 200,
	CREATED = 201,
	NO_CONTENT = 204,
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
	BAD_GATEWAY = 502,
	SERVICE_UNAVAILABLE = 503
};

class Http
{
	HttpVersion version;
	HashMap headers;
	std::string body;

public:
	Http(const std::string &raw);
	Http();

	HttpVersion getVersion() const;
	void setVersion(HttpVersion version);
	HashMap getHeaders() const;
	void setHeader(const std::string &key, const std::string &value);
	std::string getBody() const;
	void setBody(const std::string &body);
};

class Response : public Http
{
	HttpStatus status;

public:
	Response(const std::string &raw);
	HttpStatus getStatus() const;
	void setStatus(HttpStatus status);
};
class Request : public Http
{
	HttpMethod method;
	Uri uri;

public:
	Request(const std::string &raw);
	HttpMethod getMethod() const;
	void setMethod(HttpMethod method);
	Uri getUri() const;
	void setUri(const Uri &uri);
};

#endif
