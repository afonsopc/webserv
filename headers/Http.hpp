#ifndef HTTP_HPP
#define HTTP_HPP

#include <string>
#include <map>
#include <iostream>
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

#endif
