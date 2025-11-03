#include "Webserv.hpp"

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
		// check si port deja bind.si oui fds[i] = fd du meme port
		this->fds[i] = initSocket(servers[i]);
	}
	//sockets bind listen..
}

int WebServ::initSocket(struct ServerConfig &server)//classe ou struct server??
{
	int wsocket;//fd
	struct sockaddr_in sockaddr;
	socklen_t server_len;//avant unsigned int

	wsocket = socket(AF_INET, SOCK_STREAM, 0); // = IPV4, stream, 0 = TCP
	if (wsocket == -1)
		throw 3;// a voir apres
	sockaddr.sin_family = AF_INET;//tout le temps pareil
	sockaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	sockaddr.sin_port = htons(server.listen_port);// 
	server_len = sizeof(sockaddr);

	if (bind(wsocket, (struct sockaddr *)&sockaddr, server_len)!= 0)
		throw 4;
	if (listen(wsocket, 20) != 0)
		throw 5;
	std::cout << "Listening on 127.0.0.1:" << server.listen_port << std::endl;

	server.fd = wsocket;//utile??
	return wsocket;//fd du socket
}

void WebServ::initPoll()
{
	//debut epoll
	epollFd = epoll_create(0);//pk zero?? const?? mettre dans 
	if (epollFd < 0)
		throw 6;
	
	// ecouter chaque server voir si nouvelles connexion
}
/*
void data::epollStarting(server &server)//voir data struct
{
	//epoll create fait 1 fd pour orchestrer tout
	int const epollFd = epoll_create(0);//pk zero??
	if (epollFd < 0)
		thow (epollException())
	server.SetEpollfd(epollFd);
	std::vector<int> const sockets = server.getSockets();
	//parcours la liste des sockets
	for (siye_t i = 0; i < sockets.size; i++)
	{
		//ajoute a epoll les sockest a surveiller
		//donc epoll prend en charge les sockets. 
		epoll_envent event;
		event.events = EPOLLIN | EPOLLET;
		event.data.fd = sockets[i];
	if (epoll_ctl(server.getEpollFd(), EPOLL_CTL_ADD, sockets[i], &event) < 0)
	{
		perror("epoll_ctl: epollStarting()");// mettre le message d erreur dans le throw??
		throw (exeption  no server)
	}
	}
}*/