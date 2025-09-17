#include "Http.hpp"

Http::Http() {}

Http::Http(const std::string &raw)
{
    (void)raw; // Suppress unused parameter warning
    // TODO: Implement HTTP parsing
}

HttpVersion Http::getVersion() const
{
    return version;
}

void Http::setVersion(HttpVersion version)
{
    this->version = version;
}

HashMap Http::getHeaders() const
{
    return headers;
}

void Http::setHeader(const std::string &key, const std::string &value)
{
    headers.set(key, value);
}

std::string Http::getBody() const
{
    return body;
}

void Http::setBody(const std::string &body)
{
    this->body = body;
}

// Response implementations
Response::Response(const std::string &raw) : Http(raw)
{
    status = OK;
}

HttpStatus Response::getStatus() const
{
    return status;
}

void Response::setStatus(HttpStatus status)
{
    this->status = status;
}

// Request implementations
Request::Request(const std::string &raw) : Http(raw), uri("")
{
    method = GET;
}

HttpMethod Request::getMethod() const
{
    return method;
}

void Request::setMethod(HttpMethod method)
{
    this->method = method;
}

Uri Request::getUri() const
{
    return uri;
}

void Request::setUri(const Uri &uri)
{
    this->uri = uri;
}