/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: beboccas <beboccas@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/07 15:17:09 by beboccas          #+#    #+#             */
/*   Updated: 2025/11/07 15:28:28 by beboccas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// Avoid including Webserv.hpp here to avoid circular includes.
#include <string>
#include <vector>
#include <map>
#include <utility>

//struct pour la classse serveur config
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

    ServerConfig()
        : listen_port(0), server_name(), error_pages(), root(), client_max_body_size(0), locations(), fd_socket_serv(-1)
    {}
};
