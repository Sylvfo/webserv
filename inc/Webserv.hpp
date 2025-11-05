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

/*
 *	ServerConfig includes
*/
#include <string>
#include <vector>
#include <map>



#include "parsing.hpp"
#include "DataStructureWebServ.hpp"


#define MAX_EVENTS 200 //A voir

class WebServ
{
	private:
		/* data */
	public:
		WebServ(/* args */);
		~WebServ();
		bool epollWaiting();
		void initServers();
		int initSocket(struct ServerConfig &server);
		void initPoll();
		void startServers();
		void defaultConfig();
		void FileConfig();
		void free_webserv();
		WebServ parseConfigFile(int ac, char **av);

		//to do
		std::vector<ServerConfig> servers;//liste des serveurs
		std::vector<int> fds;// fd server
		int epollFd;
		std::vector<int> fdconn;// fd connections
};



#endif