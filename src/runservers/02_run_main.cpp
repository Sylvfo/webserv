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
	if (shouldShutdown())
		return false;

	struct epoll_event current_events[MAX_EVENTS];

	//int const ndfs = epoll_wait(epollFd, current_events, MAX_EVENTS, -1);
	int const ndfs = epoll_wait(epollFd, current_events, MAX_EVENTS, 1000);
	if (ndfs < 0)
	{
		if (errno == EINTR)
			return !shouldShutdown();//???
		throw  std::runtime_error("Error in epollWaiting");
	}
	for (int i = 0; i < ndfs; i++)
	{
		if ((current_events[i].events & EPOLLERR) || 
			(current_events[i].events & EPOLLHUP) || 
			!(current_events[i].events & EPOLLIN))
		{
			closeConnection(current_events[i]);
			continue;
		}
		int index = newConnection(current_events[i]);
		if (index != -1)
		{
			if (!acceptConnection(index))
				return false;
		}
		else
		{
			handleRequest(current_events[i]);
			//std::cout << "Request answered" << std::endl;
			closeConnection(current_events[i]);
		}
	}
	return !shouldShutdown();
	//return true;
}

int	WebServ::newConnection(epoll_event new_event)
{
	ConnectionData* connInfo = static_cast<ConnectionData*>(new_event.data.ptr);
	std::map<int, ConnectionData*>::iterator it = ServersConnections.find(connInfo->server_fd);
	if (it != ServersConnections.end() && (new_event.events & EPOLLIN))
	{
		//std::cout << LIGHT_RED "it is a new connection on fd" << it->first << RESET << std::endl;
		return it->second->server_index;
	}
	return -1;
}

bool WebServ::acceptConnection(int index)
{
	while (1)
	{
		int new_socket;
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		new_socket = accept(servers[index].fd_socket_serv, 
							(struct sockaddr *)&client_addr, &client_len);
		if (new_socket < 0)
		{
			//std::cout << "new connection not accepted" << std::endl;
			break;
		}
		setNonBlocking(new_socket);
		ConnectionData* connectionInfo = CreateConnection(index, new_socket);

		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET;
		event.data.ptr = connectionInfo;
		if (epoll_ctl(epollFd, EPOLL_CTL_ADD, new_socket, &event) < 0)
		{
			delete connectionInfo;
			throw std::runtime_error("epoll_ctl() for clients failed");
		}
		ClientsConnections.insert(std::make_pair(new_socket, connectionInfo));
	}
	
	return true;
}

void WebServ::closeConnection(epoll_event current_event)
{
	ConnectionData* connInfo = static_cast<ConnectionData*>(current_event.data.ptr);
	if (!connInfo)
        return;
	if (connInfo->is_server == false)
	{
		//std::cout << "connection closed fd: " << connInfo->client_fd << std::endl;
		epoll_ctl(epollFd, EPOLL_CTL_DEL, connInfo->client_fd, NULL);
		close(connInfo->client_fd);
		//std::map<int, ConnectionData*>::iterator it = ClientsConnections.find(connInfo->client_fd);
		ClientsConnections.erase(connInfo->client_fd);
		delete (connInfo);
	}
	else if (connInfo->is_server == true)
	{
		epoll_ctl(epollFd, EPOLL_CTL_DEL, connInfo->server_fd, NULL);
		//restart server
		//delete (connInfo);
	}
	//current_event.data.ptr = NULL;
}

ConnectionData* WebServ::CreateConnection(int index, int new_socket)
{
	ConnectionData* NewConnection= new ConnectionData();
	NewConnection->client_fd = new_socket;
	NewConnection->server_index = index;
	NewConnection->is_server = false;
	NewConnection->server = &servers[index];
	return (NewConnection);
}
