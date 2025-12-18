#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include "colors.hpp"
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "LocationConfig.hpp"

class ServerConfig
{
public:
    int listen_port;
    std::string server_name;
    std::map<int, std::string> error_pages;
    std::string root;
    size_t client_max_body_size;
    std::vector<LocationConfig> locations;

	int fd_socket_serv;
	struct sockaddr_in sockaddr;
	socklen_t server_len;
	std::map<int, std::string> error_code_message;
	std::map<int, std::string> default_error_html;
	std::map<std::string, std::string> mime_types;

	ServerConfig();
	ServerConfig(const ServerConfig& other);
	ServerConfig& operator=(const ServerConfig& other);
	~ServerConfig();
};

#endif
