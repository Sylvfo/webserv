#include "Webserv.hpp"

bool WebServ::epollWaiting()
{
	if (shouldShutdown())
		return false;
	struct epoll_event current_events[MAX_EVENTS];
	int const ndfs = epoll_wait(epollFd, current_events, MAX_EVENTS, 1000);
	if (ndfs < 0)
	{
		if (errno == EINTR)
			return !shouldShutdown();
		throw  std::runtime_error("Error in epollWaiting");
	}
	for (int i = 0; i < ndfs; i++)
	{
		if ((current_events[i].events & EPOLLERR) || 
			(current_events[i].events & EPOLLHUP))
		{
			closeConnection(current_events[i]);
			continue;
		}
		if (!(current_events[i].events & EPOLLIN))
			continue;
		int index = newConnection(current_events[i]);
		if (index != -1)
		{
			if (!acceptConnection(index))
				return false;
		}
		else
		{
			ConnectionData* connInfo = static_cast<ConnectionData*>(current_events[i].data.ptr);
			handleRequest(current_events[i]);
			if (connInfo && connInfo->request.request_complete)
				closeConnection(current_events[i]);
		}
	}
	return !shouldShutdown();
}

int	WebServ::newConnection(epoll_event new_event)
{
	ConnectionData* connInfo = static_cast<ConnectionData*>(new_event.data.ptr);
	std::map<int, ConnectionData*>::iterator it = ServersConnections.find(connInfo->server_fd);
	if (it != ServersConnections.end() && (new_event.events & EPOLLIN))
		return it->second->server_index;
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
			break;
		

		setNonBlocking(new_socket);
		ConnectionData* connectionInfo = createConnection(index, new_socket);

		struct epoll_event event;
		event.events = EPOLLIN;
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
		epoll_ctl(epollFd, EPOLL_CTL_DEL, connInfo->client_fd, NULL);
		close(connInfo->client_fd);
		ClientsConnections.erase(connInfo->client_fd);
		delete (connInfo);
	}
	else if (connInfo->is_server == true)
	{
		epoll_ctl(epollFd, EPOLL_CTL_DEL, connInfo->server_fd, NULL);
	}
}

ConnectionData* WebServ::createConnection(int index, int new_socket)
{
	ConnectionData* NewConnection= new ConnectionData();
	NewConnection->client_fd = new_socket;
	NewConnection->server_index = index;
	NewConnection->is_server = false;
	NewConnection->server = &servers[index];
	return (NewConnection);
}
