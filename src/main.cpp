#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>

#include "HashMap.hpp"
#include "Server.hpp"
#include "WebServ.hpp"
#include "Http.hpp"

std::vector<Server> parseConfig(const char *config_file_name)
{
    std::ifstream file(config_file_name);
    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string configContent = buffer.str();
    HashMap serverConfig1(configContent);
    std::vector<Server> servers;
    std::vector<HashMapValue> serverArray = serverConfig1.get("servers").asArray();
    for (std::vector<HashMapValue>::const_iterator it = serverArray.begin(); it != serverArray.end(); ++it)
        servers.push_back(Server(it->asObject()));
    return servers;
}

static Request loadTestRequest(void)
{
    return Request("POST /api/users HTTP/1.1\r\nHost: localhost:8080\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36\r\nAccept: application/json, text/plain, */*\r\nAccept-Language: en-US,en;q=0.9,pt;q=0.8\r\nAccept-Encoding: gzip, deflate, br\r\nContent-Type: application/json\r\nContent-Length: 156\r\nConnection: keep-alive\r\nAuthorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoxMjMsImV4cCI6MTYzMjQ2NzIwMH0\r\nCookie: session_id=abc123def456; user_pref=theme_dark\r\nReferer: https://localhost:8080/dashboard\r\nCache-Control: no-cache\r\n\r\n{ \"name\" : \"João Silva\", \"email\" : \"joao.silva@example.com\", \"age\" : 28, \"department\" : \"Engineering\", \"skills\" : [ \"JavaScript\", \"Python\", \"C++\" ], \"active\" : true }");
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return (std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl, 1);
    std::vector<Server> servers = parseConfig(argv[1]);
    WebServ webserv(servers);
    Request req = loadTestRequest();
    std::cout << "Hello, WebServ!" << std::endl;
}
