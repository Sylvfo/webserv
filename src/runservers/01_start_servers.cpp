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
			initServerSocket(servers[i], i);
	}
}

bool WebServ::checkExistingPort(int index)
{
	for (int k = 0; k < index; k++)
	{
		if (servers[k].listen_port == servers[index].listen_port)
			return true;
	}
	return false;
}

//revoir la théorie
int WebServ::initServerSocket(struct ServerConfig &server, int index)
{
	int fd_socket_servers;

	fd_socket_servers = socket(AF_INET, SOCK_STREAM, 0); // = IPV4, stream, 0 = TCP
	if (fd_socket_servers == -1)
		throw 3;// a voir apres
	
	int opt = 1;
	setsockopt(fd_socket_servers, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));//mieux comprendre

	//bind on local address		
	server.sockaddr.sin_family = AF_INET;//tout le temps pareil
	server.sockaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	server.sockaddr.sin_port = htons(server.listen_port);// 
	server.server_len = sizeof(sockaddr);
	if (bind(fd_socket_servers, (struct sockaddr *)&server.sockaddr, server.server_len)!= 0)
		throw 4;
	if (listen(fd_socket_servers, 20) != 0)
		throw 5;
	setNonBlocking(fd_socket_servers);
	std::cout << "Listening on http://" << server.server_name << ":" << server.listen_port << std::endl;
	server.fd_socket_serv = fd_socket_servers;//utile oui :)
	//std::cout << "server.fd_socket_serv " << server.fd_socket_serv << std::endl;
	ConnectionData* connInfo = new ConnectionData();
	connInfo->server_index = index;
	ServersConnections.insert(std::make_pair(fd_socket_servers, connInfo));
	return fd_socket_servers;
}


void WebServ::initPoll()
{
	this->epollFd = epoll_create(1);//pk zero?? const?? mettre dans 
	if (epollFd < 0)
		throw 6;
	std::map<int, ConnectionData*>::iterator it;
	for (it = ServersConnections.begin(); it != ServersConnections.end(); ++it)
	{
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET;
		it->second->client_fd = 0;
		it->second->server_fd = it->first;
		event.data.ptr = it->second;
		if (epoll_ctl(epollFd, EPOLL_CTL_ADD,it->first, &event) < 0)
			throw 7;
	}
}

void setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags |O_NONBLOCK);
}
