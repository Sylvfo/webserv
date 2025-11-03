#include "Webserv.hpp"

WebServ::WebServ()
{
	//epollFd = epoll_create(0);//pk zero?? const?? mettre ca ici??
	//if (epollFd < 0)
	//	throw 6;
}

WebServ::~WebServ()
{

}

void WebServ::free_webserv()
{
	std::cout << "Not integrated : Free webserv data.";
}