#include "Webserv.hpp"


WebServ::WebServ()
{
}

WebServ::WebServ(const WebServ& other)
	: servers(other.servers)
	, epollFd(other.epollFd)
{
	//la copie des maps avec pointeurs est un peu trop complexe pour moi
	(void)other;
}

WebServ& WebServ::operator=(const WebServ& other)
{
	if (this != &other)
	{
		servers = other.servers;
		epollFd = other.epollFd;
		//meme soucis avec les maps
	}
	return *this;
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
