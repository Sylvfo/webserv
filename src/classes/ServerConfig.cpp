#include "Webserv.hpp"

ServerConfig::ServerConfig()
	: listen_port(0)
	, server_name()
	, error_pages()
	, root()
	, client_max_body_size(0)
	, locations()
	, fd_socket_serv(-1)
	, sockaddr()
	, server_len(0)
	, error_code_message()
	, mime_types()
{
}

ServerConfig::ServerConfig(const ServerConfig& other)
	: listen_port(other.listen_port)
	, server_name(other.server_name)
	, error_pages(other.error_pages)
	, root(other.root)
	, client_max_body_size(other.client_max_body_size)
	, locations(other.locations)
	, fd_socket_serv(other.fd_socket_serv)
	, sockaddr(other.sockaddr)
	, server_len(other.server_len)
	, error_code_message(other.error_code_message)
	, mime_types(other.mime_types)
{
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
	if (this != &other)
	{
		listen_port = other.listen_port;
		server_name = other.server_name;
		error_pages = other.error_pages;
		root = other.root;
		client_max_body_size = other.client_max_body_size;
		locations = other.locations;
		fd_socket_serv = other.fd_socket_serv;
		sockaddr = other.sockaddr;
		server_len = other.server_len;
		error_code_message = other.error_code_message;
		mime_types = other.mime_types;
	}
	return *this;
}

ServerConfig::~ServerConfig()
{
}