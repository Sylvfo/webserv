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

bool WebServ::epollWaiting()//eventListent= // epollscanning
{
	int index = -1;//pk??
	struct epoll_event current_events[MAX_EVENTS];//struct?

	//avec epoll, va passer sur tous les fd des sockets pour voir s il se passe qqch
	int const ndfs = epoll_wait(epollFd, current_events,MAX_EVENTS, -1); // -1 infinite
	if (ndfs < 0)
		throw 8;
	for (int i = 0; i < ndfs; i++)
	{
		// nouvelle connection
		if ((index = newConnection(current_events[i])) != -1)// a definir si c est une nouvelle connection
		{
			if (!(acceptConnection(index)))
				return false;
		}
		//close connection
		else if ((current_events[i].events & EPOLLERR) || (current_events[i].events & EPOLLHUP) || !(current_events[i].events && EPOLLIN))
		{
			std::cout << "close connection" << std::endl;
			close(current_events[i].data.fd);
			return true;
		}
		else
		{
			handleRequest(index, current_events[i].data.fd);
			std::cout << "Request answered" << std::endl;
		}
	}
	return true;
}



int	WebServ::newConnection(epoll_event new_event)
{
	//std::cout << "new connection coming" << std::endl;
	for (size_t i = 0; i < fds.size(); i++)
	{
		if (new_event.data.fd == fds[i] && new_event.events == EPOLLIN)
		{
			return i;
		}	
	}
	return -1;
}

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
}
//bind / accept
