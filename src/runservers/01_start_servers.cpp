#include "Webserv.hpp"

void WebServ::startServers()
{
	this->initServers();
	this->initPoll();
}

void WebServ::initServers()
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		if (checkExistingPort(i) == false)
		{
			initServerSocket(servers[i], i);
		}
		initErroCode(servers[i]);
		initMimeTypes(servers[i]);
		initDefautlPage(servers[i]);
	}
}

bool WebServ::checkExistingPort(int index)
{
	for (int k = 0; k < index; k++)
	{
		if (servers[k].listen_port == servers[index].listen_port)
		{
			return true;
		}
	}
	return false;
}

int WebServ::initServerSocket(ServerConfig &server, int index)
{
	int fd_socket_servers;

	fd_socket_servers = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_socket_servers == -1)
		throw std::runtime_error("Error in socket server");

	int opt = 1;
	setsockopt(fd_socket_servers, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	server.sockaddr.sin_family = AF_INET;
	server.sockaddr.sin_addr.s_addr = INADDR_ANY;
	server.sockaddr.sin_port = htons(server.listen_port);
	server.server_len = sizeof(sockaddr);
	if (bind(fd_socket_servers, (struct sockaddr *)&server.sockaddr, server.server_len)!= 0)
	{
		throw std::runtime_error("bind() failed");
	}
	
	if (listen(fd_socket_servers, 1024) != 0)
	{
		throw std::runtime_error("listen() failed");
	}
	std::cout << "Listening on port http://localhost:" << server.listen_port << std::endl;
	setNonBlocking(fd_socket_servers);
	server.fd_socket_serv = fd_socket_servers;
	
	ConnectionData* connInfo = new ConnectionData();
	connInfo->server_index = index;
	ServersConnections.insert(std::make_pair(fd_socket_servers, connInfo));
	return fd_socket_servers;
}

void WebServ::initPoll()
{
	this->epollFd = epoll_create(1);
	if (epollFd < 0)
	{
		throw std::runtime_error("epoll_create() failed");
	}
	
	std::map<int, ConnectionData*>::iterator it;
	for (it = ServersConnections.begin(); it != ServersConnections.end(); ++it)
	{
		struct epoll_event event;
		event.events = EPOLLIN;
		it->second->client_fd = 0;
		it->second->server_fd = it->first;
		event.data.ptr = it->second;
		if (epoll_ctl(epollFd, EPOLL_CTL_ADD,it->first, &event) < 0)
		{
			throw std::runtime_error("epoll_ctl() for servers failed");
		}
	}
}

void setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		return;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
	}
}