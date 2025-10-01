#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <iostream>
#include "HashMap.hpp"
#include "Http.hpp"

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
		SERVICE_UNAVAILABLE = 503,
		MOVED_PERMANENTLY = 301,
		FOUND = 302,
		IM_A_TEAPOT = 418
	};

private:
	e_status status;

public:
	Response(const e_version &, const e_status &, const HashMap &headers, const std::string &body);
	e_status getStatus() const;
	void setStatus(e_status);
	std::string stringify() const;
};

#endif
