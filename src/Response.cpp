#include "Response.hpp"
#include <iostream>
#include <sstream>

int Response::getStatus() const { return (status); }
void Response::setStatus(int status) { this->status = status; }

Response::Response(const e_version &version, int status, const HashMap &headers, const std::string &body)
	: Http(version, headers, body), status(status) {}

static std::string versionToString(Http::e_version version)
{
	switch (version)
	{
	case Http::HTTP_1_0:
		return "HTTP/1.0";
	case Http::HTTP_1_1:
		return "HTTP/1.1";
	case Http::HTTP_2_0:
		return "HTTP/2.0";
	default:
		return "unknown";
	}
}

static std::string getStatusMessage(int status)
{
	switch (status)
	{
	case 100:
		return "100 Continue";
	case 101:
		return "101 Switching Protocols";
	case 102:
		return "102 Processing";
	case 103:
		return "103 Early Hints";
	case 200:
		return "200 OK";
	case 201:
		return "201 Created";
	case 202:
		return "202 Accepted";
	case 203:
		return "203 Non-Authoritative Information";
	case 204:
		return "204 No Content";
	case 205:
		return "205 Reset Content";
	case 206:
		return "206 Partial Content";
	case 207:
		return "207 Multi-Status";
	case 208:
		return "208 Already Reported";
	case 218:
		return "218 This is fine (Apache Web Server)";
	case 226:
		return "226 IM Used";
	case 300:
		return "300 Multiple Choices";
	case 301:
		return "301 Moved Permanently";
	case 302:
		return "302 Found";
	case 303:
		return "303 See Other";
	case 304:
		return "304 Not Modified";
	case 306:
		return "306 Switch Proxy";
	case 307:
		return "307 Temporary Redirect";
	case 308:
		return "308 Resume Incomplete";
	case 400:
		return "400 Bad Request";
	case 401:
		return "401 Unauthorized";
	case 402:
		return "402 Payment Required";
	case 403:
		return "403 Forbidden";
	case 404:
		return "404 Not Found";
	case 405:
		return "405 Method Not Allowed";
	case 406:
		return "406 Not Acceptable";
	case 407:
		return "407 Proxy Authentication Required";
	case 408:
		return "408 Request Timeout";
	case 409:
		return "409 Conflict";
	case 410:
		return "410 Gone";
	case 411:
		return "411 Length Required";
	case 412:
		return "412 Precondition Failed";
	case 413:
		return "413 Request Entity Too Large";
	case 414:
		return "414 Request-URI Too Long";
	case 415:
		return "415 Unsupported Media Type";
	case 416:
		return "416 Requested Range Not Satisfiable";
	case 417:
		return "417 Expectation Failed";
	case 418:
		return "418 I'm a teapot";
	case 419:
		return "419 Page Expired (Laravel Framework)";
	case 420:
		return "420 Enhance Your Calm";
	case 421:
		return "421 Misdirected Request";
	case 422:
		return "422 Unprocessable Entity";
	case 423:
		return "423 Locked";
	case 424:
		return "424 Failed Dependency";
	case 426:
		return "426 Upgrade Required";
	case 428:
		return "428 Precondition Required";
	case 429:
		return "429 Too Many Requests";
	case 431:
		return "431 Request Header Fields Too Large";
	case 440:
		return "440 Login Time-out";
	case 444:
		return "444 Connection Closed Without Response";
	case 449:
		return "449 Retry With";
	case 450:
		return "450 Blocked by Windows Parental Controls";
	case 451:
		return "451 Unavailable For Legal Reasons";
	case 494:
		return "494 Request Header Too Large";
	case 495:
		return "495 SSL Certificate Error";
	case 496:
		return "496 SSL Certificate Required";
	case 497:
		return "497 HTTP Request Sent to HTTPS Port";
	case 498:
		return "498 Invalid Token (Esri)";
	case 499:
		return "499 Client Closed Request";
	case 500:
		return "500 Internal Server Error";
	case 501:
		return "501 Not Implemented";
	case 502:
		return "502 Bad Gateway";
	case 503:
		return "503 Service Unavailable";
	case 504:
		return "504 Gateway Timeout";
	case 505:
		return "505 HTTP Version Not Supported";
	case 506:
		return "506 Variant Also Negotiates";
	case 507:
		return "507 Insufficient Storage";
	case 508:
		return "508 Loop Detected";
	case 509:
		return "509 Bandwidth Limit Exceeded";
	case 510:
		return "510 Not Extended";
	case 511:
		return "511 Network Authentication Required";
	case 520:
		return "520 Unknown Error";
	case 521:
		return "521 Web Server Is Down";
	case 522:
		return "522 Connection Timed Out";
	case 523:
		return "523 Origin Is Unreachable";
	case 524:
		return "524 A Timeout Occurred";
	case 525:
		return "525 SSL Handshake Failed";
	case 526:
		return "526 Invalid SSL Certificate";
	case 527:
		return "527 Railgun Listener to Origin Error";
	case 530:
		return "530 Origin DNS Error";
	case 598:
		return "598 Network Read Timeout Error";
	}
	return "XXX Unknown";
}

std::string Response::stringify(const std::string &method, const std::string &path) const
{
	if (!method.empty())
	{
		std::cout << "Request:  " << method << " " << path << std::endl;
		std::cout << "Response: " << getStatusMessage(getStatus()) << std::endl
				  << std::endl;
	}
	std::string str;
	str += versionToString(getVersion()) + " " + getStatusMessage(getStatus()) + "\r\n";
	str += getHeaders().headerify() + "\r\n\r\n";
	str += getBody();
	return (str);
}
