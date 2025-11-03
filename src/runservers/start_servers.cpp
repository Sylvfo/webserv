#include "Webserv.hpp"

void WebServ::startServers()
{
	std::cout << "Starting servers..." << std::endl;

	this->initServ();
	this->initPoll();
}

void WebServ::initServ()
{
	std::cout << "Init Serv " << std::endl;

	//for each server
	//sockets bind listen..
}

void WebServ::initPoll()
{
	//debut epoll
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