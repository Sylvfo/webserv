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
#include <sys/stat.h>
//getaddrinfo()
//freeaddrinfo()
//gai_strerro()
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <signal.h>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cstdlib>

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "RequestHeader.hpp"
#include "colors.hpp"

#define MAX_EVENTS 100 //A voir

struct ConnectionData
{
    int client_fd;
	int server_fd;
    int server_index;  // Index dans servers[]
    ServerConfig* server; // Pointeur direct
};

class WebServ
{
	private:
		std::vector<ServerConfig> servers;
		std::vector<int> fd_servers;//ancien fds
		int epollFd; // file desciptor epoll. fds[0] ??
		std::map<int, ConnectionData*> ServersConnections;
		std::map<int, ConnectionData*> ClientsConnections;
		//std::map<int, ConnectionData*> connections; //client connection??
		//std::vector<int> ports;

	public:
		WebServ();
		~WebServ();

		//parsing
		void defaultConfig();
		void FileConfig();
		/* Server Config related functions */	 			/* **************************************************** */
		ServerConfig getServer(int index);	 				/* Can be searched by port or by server name. 			*/
		ServerConfig getServer(std::string hostReq);		/* Can be searched by port or by server name. 			*/
		void addServer(ServerConfig config); 				/* Add a server to the ServerConfig list 				*/
															/*														*/
		std::vector<ServerConfig> getServerList();			/* Get the entire server list as vector<ServerConfig>	*/
		void parseConfig(std::string path);					/* Parse the config file .conf and add each server		*/
		void printConfig();									/* Print the entire config (DEBUG ONLY)					*/
		/* End of the server config functions */			/* **************************************************** */

		//start servers
		void initServers();
		int initServerSocket(struct ServerConfig &server);
		bool checkExistingPort(int index);
		void initPoll();
		void startServers();

		//epoll
		bool	epollWaiting();
		int		newConnection(epoll_event new_event);
		ConnectionData* CreateConnection(int index, int new_socket);
		bool	acceptConnection(int index);
		void	closeConnection(epoll_event current_event);

		//handle request
		void handleRequest(epoll_event current_event);
		//free
		void free_webserv();

	//	handel
		void printfds();
	//	void printfdconn();//sert a rien??
		void printepollwait(struct epoll_event *csurrent_events, int ndfs);

};

void setNonBlocking(int fd);
void handleSignInt(int sign);

void printcurrentevent(struct epoll_event *current_events, int ndfs);

#endif
