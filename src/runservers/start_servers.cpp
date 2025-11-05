#include "Webserv.hpp"

//void setNonBlocking

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
		// check si port deja bind.si oui pas creer nlle socket
		this->fds[i] = initSocket(servers[i]);
	}
}

//voir setsocketopt() pour non bloquant
//fcntl pour non blocant. opt NON BLOCKING??
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
	for (size_t i; i < fds.size(); i++) //dans tous les fd
	{
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET;
		event.data.fd = fds[i];
		if (epoll_ctl(epollFd, EPOLL_CTL_ADD,fds[i], &event) < 0)
			throw 7;
	}
}

bool WebServ::epollWaiting()//eventListent= // epollscanning
{
	//voir fonction Loic
	int index = -1;//pk??
	epoll_event currents_events[MAX_EVENTS];//struct?

	//avec epoll, va passer sur tous les fd des sockets pour voir s il se passe qqch
	int const ndfs = epoll_wait(epollFd, currents_events,MAX_EVENTS, -1); // -1 infinite
	if (ndfs < 0)
		throw 8;
	for (int i = 0; i < ndfs; i++)
	{
		// nouvelle connection
		if ((index = new connextion ) != -1)// a definir si c est une nouvelle connection
		{
			if (!acceptConnection(...))
				return false;
		}
		//close connection
		else if ((currents_events[i].events & EPOLLERR) || (currents_events[i].events & EPOLLHUP) || !(currents_events[i].events && EPOLLIN))
		{
			std::cout << "close connection" << std::endl;
			close(currents_events[i].data.fd);
			return true;
		}
		else
		{
		//	handleRequest(server, currents_events[i].data.fd);// a definir suite du projet
			std::cout << "Request ansered" << std::endl;
		}

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