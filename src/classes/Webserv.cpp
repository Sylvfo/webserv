#include "Webserv.hpp"

WebServ::WebServ()
{
	//epollFd = epoll_create(0);//pk zero?? const?? mettre ca ici??
	//if (epollFd < 0)
	//	throw 6;
}

WebServ::~WebServ()
{
	servers.clear();
	close(epollFd);
	std::map<int, ConnectionData*>::iterator it;
	for (it = ServersConnections.begin(); it != ServersConnections.end(); ++it) {
		delete it->second;
		it->second = NULL;
	}
	ServersConnections.clear();
	for (it = ClientsConnections.begin(); it != ClientsConnections.end(); ++it) {
		delete it->second;
		it->second = NULL;
	}
	ClientsConnections.clear();
	// a modifier
	/*for (size_t i = 0; i < fd_servers.size(); i++)
	{
		close(fd_servers[i]);
	}*/
	//ServersConnections
	//fd_servers.clear();
//	fd_servers.clear();
	std::cout << "destr called";
}

void WebServ::free_webserv()
{
	close(epollFd);
	std::cout << "Not integrated : Free webserv data.";
}