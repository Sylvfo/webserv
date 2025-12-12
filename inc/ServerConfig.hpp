/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bschmid <bschmid@student.42lausanne.ch>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/12 01:30:25 by bschmid           #+#    #+#             */
/*   Updated: 2025/12/12 01:31:33 by bschmid          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <utility>

// added so would compile but not used in header parsing
#include <netinet/in.h>
#include <sys/socket.h>

//struct pour la classe serveur config
struct LocationConfig
{
    std::string path;
    std::vector<std::string> methods;
    bool autoindex;
    std::string index;
    std::string root;
    std::string upload_path;
    std::string cgi_path;
    std::vector<std::pair<int, std::string> > returns;

    LocationConfig()
        : path(), methods(), autoindex(false), index(), root(), upload_path(), cgi_path(), returns()
    {}
};

//faire une classe ServerConfig
struct ServerConfig
{
    int listen_port;
    std::string server_name;
    std::map<int, std::string> error_pages;
    std::string root;
    long client_max_body_size;
    std::vector<LocationConfig> locations;

	//pour start server
	int fd_socket_serv;//a voir si c est utile
	struct sockaddr_in sockaddr;
	socklen_t server_len;
	std::map<int, std::string> error_code_message;
	std::map<std::string, std::string> mime_types;

	ServerConfig()
        : listen_port(0), server_name(), error_pages(), root(), client_max_body_size(0), locations(), fd_socket_serv(-1)
    {}
};

