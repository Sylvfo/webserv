#include "Webserv.hpp"

void WebServ::printfds()
{
	for (size_t i = 0; i < fds.size(); i++)
		std::cout << "fds " << i << ": " << fds[i] << std::endl;
}

void printcurrentevent(struct epoll_event *current_events, int ndfs)
{
	std::cout << "ndfs: " << ndfs << std::endl;
	std::cout << "current event fds" << std::endl;
	for (int i = 0; i < ndfs; i++)
		std::cout << current_events[i].data.fd << " ";
	std::cout << std::endl;
}
/*
void WebServ::printfdconn()
{
	for (size_t i = 0; i < fdconn.size(); i++)
		std::cout << "fdconn " << i << ": " << fdconn[i] << std::endl;
}*/

void WebServ::printepollwait(struct epoll_event *current_events, int ndfs)
{
	std::cout << LIGHT_TURQUOISE  " in epoll waiting" << std::endl;
	printcurrentevent(current_events, ndfs);
	printfds();
	//printfdconn();

	std::cout << RESET << std::endl;
}