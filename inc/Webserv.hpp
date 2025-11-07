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
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"

#define MAX_EVENTS 100 //A voir

class WebServ
{
	private:
		std::vector<ServerConfig> servers;
	public:
		WebServ();
		~WebServ();

		//parsing
		void defaultConfig();
		void FileConfig();

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
		

		/* Server Config related functions */ 		/* **************************************************** */
		ServerConfig getServer(int index); 			/* Can be searched by port or by server name. 			*/
		void addServer(ServerConfig config); 		/* Add a server to the ServerConfig list 				*/
													/*														*/
		std::vector<ServerConfig> getServerList();	/* Get the entire server list as vector<ServerConfig>	*/
		void parseConfig(std::string path);			/* Parse the config file .conf and add each server		*/
		void printConfig();							/* Print the entire config (DEBUG ONLY)					*/
		/* End of the server config functions */	/* **************************************************** */


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