/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bschmid <bschmid@student.42lausanne.ch>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2025/12/17 23:28:11 by bschmid          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

class LocationConfig
{
public:
    std::string path;
    std::vector<std::string> methods;
    bool autoindex;
    std::string index;
    std::string root;
    std::string upload_path;
    std::string cgi_path;
    std::vector<std::pair<int, std::string> > returns;

    LocationConfig();
    LocationConfig(const LocationConfig& other);
    LocationConfig& operator=(const LocationConfig& other);
    ~LocationConfig();
};

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
