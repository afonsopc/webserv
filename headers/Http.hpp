#ifndef HTTP_HPP
#define HTTP_HPP

#include <string>
#include <map>
#include "HashMap.hpp"

class Http
{
public:
	enum e_version
	{
		HTTP_1_0,
		HTTP_1_1,
		HTTP_2_0,
		UNKNOWN_VERSION
	};

private:
	e_version version;
	HashMap headers;
	std::string body;

public:
	Http(e_version version, const HashMap &headers, const std::string &body);
	e_version getVersion() const;
	void setVersion(e_version version);
	HashMap getHeaders() const;
	void setHeader(const std::string &key, const std::string &value);
	std::string getBody() const;
	void setBody(const std::string &body);
};

class Response : public Http
{
public:
	enum e_status
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

private:
	e_status status;

public:
	e_status getStatus() const;
	void setStatus(e_status status);
};

class Request : public Http
{
public:
	enum e_method
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

private:
	e_method method;
	std::string path;

public:
	Request(const std::string &raw);
	e_method getMethod() const;
	void setMethod(e_method method);
	std::string getPath() const;
	void setPath(const std::string &path);
};

#endif
