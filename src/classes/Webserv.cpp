#include "Webserv.hpp"

WebServ::WebServ()
{
	//epollFd = epoll_create(0);//pk zero?? const?? mettre ca ici??
	//if (epollFd < 0)
	//	throw 6;
}

WebServ::~WebServ()
{
	servers.empty();
	close(epollFd);
	for (size_t i = 0; i < fd_servers.size(); i++)
		close(fd_servers[i]);
	fd_servers.empty();
//	fd_servers.clear();
	std::cout << "destr called";
}

void WebServ::free_webserv()
{
	close(epollFd);
	std::cout << "Not integrated : Free webserv data.";
}