#pragma once

#include "Webserv.hpp"

//put it in server or Webserv
class request
{
	//to do
	
	//canonical form
	
};



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

    LocationConfig(): autoindex(false) {}
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

    ServerConfig(): listen_port(0), client_max_body_size(0) {}
};

struct Config
{
    std::vector<ServerConfig> servers;
};