#include "Http.hpp"

Http::e_version Http::getVersion() const { return (version); }
void Http::setVersion(Http::e_version version) { this->version = version; }
HashMap Http::getHeaders() const { return (headers); }
void Http::setHeader(const std::string &key, const std::string &value) { headers.set(key, value); }
std::string Http::getBody() const { return (body); }
void Http::setBody(const std::string &body) { this->body = body; }

Response::e_status Response::getStatus() const { return (status); }
void Response::setStatus(Response::e_status status) { this->status = status; }

Request::e_method Request::getMethod() const { return (method); }
void Request::setMethod(Request::e_method method) { this->method = method; }
std::string Request::getPath() const { return (path); }
void Request::setPath(const std::string &path) { this->path = path; }
