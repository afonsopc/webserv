#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>
#include <iostream>
#include "HashMap.hpp"
#include "Http.hpp"

class Response : public Http
{
private:
	int status;

public:
	Response(const e_version &, int status, const HashMap &headers, const std::string &body);
	int getStatus() const;
	void setStatus(int);
	std::string stringify(const std::string &method = "", const std::string &path = "") const;
};

#endif
