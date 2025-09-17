#ifndef HTTP_INTERFACE_HPP
#define HTTP_INTERFACE_HPP

#include <string>
#include <map>

class Http
{
public:
	virtual ~Http() {}
	virtual std::string toString() const = 0;
	virtual void setHeader(const std::string &key, const std::string &value) = 0;
	virtual const std::map<std::string, std::string> &getHeaders() const = 0;
};

#endif