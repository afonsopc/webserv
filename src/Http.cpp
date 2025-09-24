#include "Http.hpp"

Http::e_version Http::getVersion() const { return (version); }
void Http::setVersion(Http::e_version version) { this->version = version; }
HashMap Http::getHeaders() const { return (headers); }
void Http::setHeader(const std::string &key, const std::string &value) { headers.set(key, value); }
std::string Http::getBody() const { return (body); }
void Http::setBody(const std::string &body) { this->body = body; }

Http::Http(e_version version, const HashMap &headers, const std::string &body)
	: version(version), headers(headers), body(body) {}
