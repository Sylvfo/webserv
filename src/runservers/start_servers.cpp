#include "Webserv.hpp"

void setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags |O_NONBLOCK);
}

void WebServ::startServers()
{
	std::cout << "Starting servers..." << std::endl;

	this->initServers();
	this->initPoll();
}

void WebServ::initServers()
{
	std::cout << "Init Serv " << std::endl;
	
	for (size_t i = 0; i < servers.size(); i++)
	{
		if (checkExistingPort(i) == false)
			this->fds[i] = initSocket(servers[i]);
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

int WebServ::initSocket(struct ServerConfig &server)
{
	int wsocket;//fd_socket_servers

	wsocket = socket(AF_INET, SOCK_STREAM, 0); // = IPV4, stream, 0 = TCP
	if (wsocket == -1)
		throw 3;// a voir apres
	
	int opt = 1;
	setsockopt(wsocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));//mieux comprendre

	//bind on local address		
	server.sockaddr.sin_family = AF_INET;//tout le temps pareil
	server.sockaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	server.sockaddr.sin_port = htons(server.listen_port);// 
	server.server_len = sizeof(sockaddr);
	if (bind(wsocket, (struct sockaddr *)&server.sockaddr, server.server_len)!= 0)
		throw 4;
	if (listen(wsocket, 20) != 0)
		throw 5;
	setNonBlocking(wsocket);
	std::cout << "Listening on 127.0.0.1:" << server.listen_port << std::endl;
	server.fd_socket_serv = wsocket;//utile?
	return wsocket;//fd du socket
}

void WebServ::initPoll()
{
	epollFd = epoll_create(0);//pk zero?? const?? mettre dans 
	if (epollFd < 0)
		throw 6;
	for (size_t i = 0; i < fds.size(); i++)
	{
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET;
		event.data.fd = fds[i];
		if (epoll_ctl(epollFd, EPOLL_CTL_ADD,fds[i], &event) < 0)
			throw 7;
	}
}

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