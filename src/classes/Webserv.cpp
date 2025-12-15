#include "Webserv.hpp"


WebServ::WebServ()
{
}

WebServ::~WebServ()
{
	servers.clear();
	close(epollFd);
	std::map<int, ConnectionData*>::iterator it;
	for (it = ServersConnections.begin(); it != ServersConnections.end(); ++it) {
		close(it->first);
		delete it->second;
		it->second = NULL;
	}
	ServersConnections.clear();
	for (it = ClientsConnections.begin(); it != ClientsConnections.end(); ++it) {
		delete it->second;
		it->second = NULL;
	}
	ClientsConnections.clear();
	//all fd closed?
	//All children closed??
	std::cout << "destr called";
}
