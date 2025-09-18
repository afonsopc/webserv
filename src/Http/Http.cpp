#include "Http.hpp"

Http::Http(e_version version, const HashMap &headers, const std::string &body)
    : version(version), headers(headers), body(body)
{
}
