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
			fd_servers.push_back(initServerSocket(servers[i]));
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

int WebServ::initServerSocket(struct ServerConfig &server)
{
	int fd_socket_servers;//fd_socket_servers

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
	std::cout << "Listening on http://127.0.0.1:" << server.listen_port << std::endl;
	server.fd_socket_serv = fd_socket_servers;//utile oui :)
	return fd_socket_servers;
}

void WebServ::initPoll()
{
	this->epollFd = epoll_create(1);//pk zero?? const?? mettre dans 
	if (epollFd < 0)
		throw 6;
	for (size_t i = 0; i < fd_servers.size(); i++)
	{
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET;
		ConnectionData* connInfo = new ConnectionData();
		connInfo->client_fd = 0;
		connInfo->server_fd = fd_servers[i];
		//connInfo->server = &servers[i];
		event.data.ptr = connInfo;
		if (epoll_ctl(epollFd, EPOLL_CTL_ADD,fd_servers[i], &event) < 0)
			throw 7;
	}
}

void setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags |O_NONBLOCK);
}

/*
void WebServ::initPoll()
{
	this->epollFd = epoll_create(1);//pk zero?? const?? mettre dans 
	if (epollFd < 0)
		throw 6;
	for (size_t i = 0; i < fds.size(); i++)
	{
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET;
		event.data.fd = fds[i];
		
		//event.data.u32 = i;
		//event.data.ptr = NULL;
		if (epoll_ctl(epollFd, EPOLL_CTL_ADD,fds[i], &event) < 0)
			throw 7;
	}

*/

/*

	fcntl
	File CoNtroL
	manipuler les descripteur de fichiers
	change le comportement d une socket dj ouvert (ou file)

	int fcntl(int fd, int cmd, ... args)

	rendre un fd non bloquant:

	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

*/