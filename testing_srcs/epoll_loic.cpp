voir setsocketopt() pour non bloquant
fcntl pour non blocant. opt NON BLOCKING??

//start 1 poll?
//creer les sockets d entrees sur tous les servers (sites) du config files
//start servers
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
}

//threads dans epoll

// a mettre dans la main loop dans le main
bool data::epollWaiting(server &Server)
{
	int index = -1;
	epoll_event events[MAX_EVENTS];

	//epoll  renvoie la liste des fd ou il y a eu un event. 

	int const ndfs = epoll_wait(server.getEpollFd(), events, MAX_EVENTS, -1);
	if (ndfs < 0)
	{
		throw error
	}
	// nouvelle connection
	for (int i = 0; i < ndfs; i++)
	{
		if ((index = server.newConnection(event[i].data.fd)) != -1)
		{
			if (!acceptConnection(server, index)) //renvoie une socket
				return false;
		}
	}
	// close connection
	else if ((event[i].events & EPOLLERR)s || (events[i].events & EPOLLHUP))
	|| (!(events[i].events && EPOLLIN))
	{
		std::cout << "close connection" << std::endl;
		close(events[i].data.fd);
		return true;
	}
	// traite requete
	else
	{
		readRequest(server, events[i].data.fd);
		std::cout << "Request ansered" << std::endl;
	}
}

int main(int ac, char **argv)
{
	// check args
	try
	{
		parsConfigFile();
		startServers();//(epoll starting);

		while(1)
		{
			data.epollWaiting();
		}
	}
	catch
	{

	}
	
}