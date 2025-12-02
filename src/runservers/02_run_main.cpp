#include "Webserv.hpp"
/*

enum EPOLL_EVENTS
  {
    EPOLLIN = 0x001, //The associated file is available for read(2) operations
#define EPOLLIN EPOLLIN
    EPOLLPRI = 0x002,
#define EPOLLPRI EPOLLPRI
    EPOLLOUT = 0x004,
#define EPOLLOUT EPOLLOUT // The associated file is available for write(2) operations.
    EPOLLRDNORM = 0x040,
#define EPOLLRDNORM EPOLLRDNORM
    EPOLLRDBAND = 0x080,
#define EPOLLRDBAND EPOLLRDBAND
    EPOLLWRNORM = 0x100,
#define EPOLLWRNORM EPOLLWRNORM
    EPOLLWRBAND = 0x200,
#define EPOLLWRBAND EPOLLWRBAND
    EPOLLMSG = 0x400,
#define EPOLLMSG EPOLLMSG
    EPOLLERR = 0x008,
#define EPOLLERR EPOLLERR
    EPOLLHUP = 0x010,
#define EPOLLHUP EPOLLHUP
    EPOLLRDHUP = 0x2000,
#define EPOLLRDHUP EPOLLRDHUP
    EPOLLEXCLUSIVE = 1u << 28,
#define EPOLLEXCLUSIVE EPOLLEXCLUSIVE
    EPOLLWAKEUP = 1u << 29,
#define EPOLLWAKEUP EPOLLWAKEUP
    EPOLLONESHOT = 1u << 30,
#define EPOLLONESHOT EPOLLONESHOT
    EPOLLET = 1u << 31
#define EPOLLET EPOLLET
  };

  EPOLLERR
Error condition happened on the associated file descriptor.
This event is also reported for the write end of a pipe
when the read end has been closed.

epoll_wait(2) will always report for this event; it is not
necessary to set it in events when calling epoll_ctl().


typedef union epoll_data
{
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

struct epoll_event
{
  uint32_t events;	 Epoll events 
  epoll_data_t data;	 User data variable 
} __EPOLL_PACKED;*/


#include "Webserv.hpp"

bool WebServ::epollWaiting()
{
    int index = -1;
    struct epoll_event current_events[MAX_EVENTS];

    int const ndfs = epoll_wait(epollFd, current_events, MAX_EVENTS, -1);
    if (ndfs < 0)
        throw 8;
        
    std::cout << "Epollfd: " << epollFd << std::endl;
    
    for (int i = 0; i < ndfs; i++)
    {
        // ✅ Récupérer ConnectionInfo
        ConnectionInfo* connInfo = static_cast<ConnectionInfo*>(current_events[i].data.ptr);
        
       // if (connInfo)
        std::cout << "fd current event client : " << connInfo->client_fd << std::endl;
        //else
        std::cout << "fd current event : server socket"  << connInfo->server_fd  << std::endl;
        
        // Check errors
        if ((current_events[i].events & EPOLLERR) || 
            (current_events[i].events & EPOLLHUP) || 
            !(current_events[i].events & EPOLLIN))
        {
            std::cout << "close connection in error" << std::endl;
            if (connInfo)
                closeConnection(connInfo->client_fd);
            continue;
        }
        
        // Nouvelle connection (connInfo == NULL pour server sockets)
        index = newConnection(current_events[i]);
        if (index != -1)
        {
            if (!acceptConnection(index))
                return false;
        }
        else//if (connInfo) // ✅ Vérifier que ce n'est pas NULL
        {
            handleRequest(current_events[i]);
            std::cout << "Request answered" << std::endl;
            closeConnection(connInfo->client_fd);
        }
    }
    return true;
}

int	WebServ::newConnection(epoll_event new_event)
{
	ConnectionInfo* connInfo = static_cast<ConnectionInfo*>(new_event.data.ptr);
	//std::cout << "new connection coming" << std::endl;
	for (size_t i = 0; i < fds.size(); i++)
	{
		if (connInfo->server_fd == fds[i] && (new_event.events & EPOLLIN)) //see existing connections...
		{
			std::cout << LIGHT_RED "it is a new connection" RESET << std::endl;
			return i;
		}	
	}
	std::cout << "Return -1" << std::endl;
	return -1;
}

/*
int	WebServ::newConnection(epoll_event new_event)
{
	//std::cout << "new connection coming" << std::endl;
	for (size_t i = 0; i < fds.size(); i++)
	{
		if (new_event.data.fd == fds[i] && (new_event.events & EPOLLIN)) //see existing connections...
		{
			std::cout << LIGHT_RED "it is a new connection" RESET << std::endl;
			return i;
		}	
	}
	std::cout << "Return -1" << std::endl;
	return -1;
}

*/

/*
int WebServ::newConnection(epoll_event new_event)
{
    // ✅ Les server sockets ont ptr == NULL
    if (new_event.data.ptr != NULL)
    {
        std::cout << "Return -1" << std::endl;
        return -1; // C'est un client socket
    }
    
    // ✅ C'est un server socket - trouver son index
    // On doit comparer avec les fds des serveurs
    // PROBLÈME: on ne peut pas accéder au fd avec data.ptr
    // SOLUTION: utiliser data.u32 pour stocker l'index du serveur
    std::cout << LIGHT_RED "it is a new connection" RESET << std::endl;
    
    // Temporaire: retourner 0 (premier serveur)
    // À améliorer: stocker l'index dans initServers()
    return 0;
}*/

bool WebServ::acceptConnection(int index)
{
    int new_socket;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

	std::cout << "in accept connection index = " << index << std::endl;
    
    new_socket = accept(servers[index].fd_socket_serv, 
                       (struct sockaddr *)&client_addr, &client_len);
    if (new_socket < 0)
    {
        std::cout << "new connection not accepted" << std::endl;
        return false;
    }
    
    setNonBlocking(new_socket);
    
    // ✅ Créer ConnectionInfo
    ConnectionInfo* connInfo = new ConnectionInfo();
    connInfo->client_fd = new_socket;
    connInfo->server_index = index;
    connInfo->server = &servers[index];
    connections[new_socket] = connInfo;

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.ptr = connInfo; // ✅ CORRECTION: stocker connInfo, pas &servers[index]
    
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, new_socket, &event) < 0)
    {
        delete connInfo;
        connections.erase(new_socket);
        throw 9;
    }
    
    std::cout << "new connection accepted fd: " << new_socket << std::endl;
    return true;
}

void WebServ::closeConnection(int fd)
{
    // ✅ Nettoyer la mémoire
    std::map<int, ConnectionInfo*>::iterator it = connections.find(fd);
    if (it != connections.end())
    {
        delete it->second;
        connections.erase(it);
    }
    
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    std::cout << "connection closed fd: " << fd << std::endl;
}
/*
bool WebServ::epollWaiting()//eventListent= // epollscanning
{
	int index = -1;//pk??
	struct epoll_event current_events[MAX_EVENTS];//struct?

	//avec epoll, va passer sur tous les fd des sockets pour voir s il se passe qqch
	int const ndfs = epoll_wait(epollFd, current_events, MAX_EVENTS, -1); // -1 infinite
	if (ndfs < 0)
		throw 8;
	std::cout << "Epollfd: " << epollFd <<  std::endl;
//	printepollwait(current_events, ndfs);
	for (int i = 0; i < ndfs; i++)
	{
		std::cout << "fd current event : " << current_events[i].data.fd <<  std::endl;
		
		//check errors
		if ((current_events[i].events & EPOLLERR) || 
            (current_events[i].events & EPOLLHUP) || 
            !(current_events[i].events & EPOLLIN))
        {
            std::cout << "close connection" << std::endl;
          //  closeConnection(current_events[i].data.fd);
			
		//	close(current_events[i].data.fd);
            continue;//??
        }
		
		//close connection
		else if ((current_events[i].events & EPOLLERR) || (current_events[i].events & EPOLLHUP) || !(current_events[i].events && EPOLLIN))
		{
			std::cout << "close connection" << std::endl;
			close(current_events[i].data.fd);
			return true;
		}
		
		// nouvelle connection
		index = newConnection(current_events[i]);
		if (index != -1)// a definir si c est une nouvelle connection
		{
			if (!(acceptConnection(index)))
				return false;
		}
		//else if (connections)
		else
		{
			// todoparsing why they are empty request????
			handleRequest(current_events[i]);
			std::cout << "Request answered" << std::endl;
		//	closeConnection(current_events[i].data.fd);
		}
	}
	return true;
}

int	WebServ::newConnection(epoll_event new_event)
{
	//std::cout << "new connection coming" << std::endl;
	for (size_t i = 0; i < fds.size(); i++)
	{
		if (new_event.data.fd == fds[i] && (new_event.events & EPOLLIN)) //see existing connections...
		{
			std::cout << LIGHT_RED "it is a new connection" RESET << std::endl;
			return i;
		}	
	}
	std::cout << "Return -1" << std::endl;
	return -1;
}



bool WebServ::acceptConnection(int index)
{
    int new_socket;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    new_socket = accept(servers[index].fd_socket_serv, 
                       (struct sockaddr *)&client_addr, &client_len);
    if (new_socket < 0)
    {
        std::cout << "new connection not accepted" << std::endl;
        return false;
    }
    
    setNonBlocking(new_socket);
   // fdconn.push_back(new_socket);
    
	ConnectionInfo* connInfo = new ConnectionInfo();
    connInfo->client_fd = new_socket;
    connInfo->server_index = index;
    connInfo->server = &servers[index];
    connections[new_socket] = connInfo;


    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;  // Remove EPOLLOUT initially
    //event.data.fd = new_socket;
	//event.data.ptr = connInfo;
	//event.data.fd = new_socket;

	event.data.ptr = &servers[index];
    
    //if (epoll_ctl(epollFd, EPOLL_CTL_ADD, new_socket, &event) < 0)
    //    throw 9;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, new_socket, &event) < 0)
    {
        delete connInfo;
        connections.erase(new_socket);
        throw 9;
    }
    
    std::cout << "new connection accepted fd: " << new_socket << std::endl;
    return true;
}

void WebServ::closeConnection(int fd)
{
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);

//	fdconn.erase(fdconn.search(fd));
    // Remove from fdconn vector
//    fdconn->erase(std::remove(fdconn.begin(), fdconn.end(), fd), fdconn.end());
	std::cout << "connection closed fd: " << fd << std::endl;
}*/




/*
bool	WebServ::acceptConnection(int index)
{
	//create new socket, set it 
	int new_socket;
	struct sockaddr_in client_addr;//mettre dans data struct???
	socklen_t client_len = sizeof(client_addr); //mettre dans data struct???
	
	new_socket = accept(servers[index].fd_socket_serv, (struct sockaddr *)&client_addr, &client_len);
	if (new_socket < 0)
	{
		std::cout << "new connection not accepted " << std::endl;
		return false;
	}
	setNonBlocking(new_socket);
	fdconn.push_back(new_socket);
	//CREATE EVENT
	//socketpair,
	struct epoll_event event;
	event.events = EPOLLIN | EPOLLET | EPOLLOUT;//?? QUELS FLAGS????
	event.data.fd = new_socket;//!!!!! A LIER AVEC fdconn
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD,new_socket, &event) < 0) //PAREIL FAUX
		throw 9;
	std::cout << "new connection accepted" << std::endl;
	return true;
}*/
//bind / accept