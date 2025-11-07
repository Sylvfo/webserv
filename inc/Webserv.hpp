#ifndef WEBSERV_HPP
#define WEBSERV_HPP

//librairies
#include <iostream>
#include <arpa/inet.h>
//htons htonl...

#include <sys/types.h>
#include <sys/socket.h>
//sockets()
//socketpain()
//accept()
//listen()
//send()
//recv()
//bind()
//connect
//setsockopt()

#include <sys/select.h>
//select()
/*
#include <poll.h>
//poll()*/
#include <sys/epoll.h>
//epoll()

//#include <sys/event.h>
// kqueue()
// kevent()

//(#include <sys/types.h>)
//(#include <sys/socket.h>)
#include <netdb.h>
//getaddrinfo()
//freeaddrinfo()
//gai_strerro()
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <signal.h>


#include "parsing.hpp"
#include "DataStructureWebServ.hpp"
#include "HttpRequest.hpp"

#define MAX_EVENTS 100 //A voir

class WebServ
{
	private:
		
	public:
		WebServ();
		~WebServ();

		//parsing
		void defaultConfig();
		void FileConfig();
		WebServ parseConfigFile(int ac, char **av);

		//start servers
		void initServers();
		int initSocket(struct ServerConfig &server);
		bool checkExistingPort(int index);
		void initPoll();
		void startServers();
		
		//epoll
		bool epollWaiting();
		int		newConnection(epoll_event new_event);
		bool	acceptConnection(int index);
		//free
		void free_webserv();
		
		std::vector<ServerConfig> servers;//liste des serveurs
		//comment on sait quelle fds est en lien avec quel serveur??
		std::vector<int> ports;
		std::vector<int> fds;// fd server
		int epollFd;
		std::vector<int> fdconn;// fd connections
	//	handel

};

void setNonBlocking(int fd);
void handleSignInt(int sign);

#endif