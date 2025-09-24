#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <iostream>
#include "HashMap.hpp"
#include "Http.hpp"

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

	friend std::ostream &operator<<(std::ostream &os, const Request &request);
};

#endif
