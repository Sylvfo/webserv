#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <sys/stat.h>
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
#include "CGI.hpp"
#include "colors.hpp"

#define MAX_EVENTS 300 //A voir

struct ConnectionData
{
    int client_fd;
	int server_fd;//pourrait servir pour virtual host
	bool is_server;
    int server_index;
    ServerConfig* server;
	HttpRequest request;
};

class WebServ
{
	private:
		std::vector<ServerConfig> servers;
		int epollFd;
		std::map<int, ConnectionData*> ServersConnections;
		std::map<int, ConnectionData*> ClientsConnections;
		static volatile sig_atomic_t g_shutdown;

	public:
		WebServ();
		WebServ(const WebServ& other);
		WebServ& operator=(const WebServ& other);
		~WebServ();

		//parsing
		void defaultConfig();
		void FileConfig();
		/* Server Config related functions */	 			/* **************************************************** */
		//ServerConfig getServer(int index);	 				/* Can be searched by port or by server name. 			*/
		//ServerConfig getServer(std::string hostReq);		/* Can be searched by port or by server name. 			*/
		void addServer(ServerConfig config); 				/* Add a server to the ServerConfig list 				*/
															/*														*/
		std::vector<ServerConfig> getServerList();			/* Get the entire server list as vector<ServerConfig>	*/
		void parseConfig(std::string path);					/* Parse the config file .conf and add each server		*/
		void printConfig();									/* Print the entire config (DEBUG ONLY)					*/
		/* End of the server config functions */			/* **************************************************** */

		//start servers
		void initServers();
		int initServerSocket(ServerConfig &server, int index);
		bool checkExistingPort(int index);
		void initPoll();
		void startServers();
		void initErroCode(ServerConfig &server);
		void initMimeTypes(ServerConfig &server);
		void initDefautlPage(ServerConfig &server);
		//epoll
		bool	epollWaiting();
		int		newConnection(epoll_event new_event);
		ConnectionData* CreateConnection(int index, int new_socket);
		bool	acceptConnection(int index);
		void	closeConnection(epoll_event current_event);

		//handle request
		void handleRequest(epoll_event current_event);

	//	handel
		void printfds();
		void printepollwait(struct epoll_event *csurrent_events, int ndfs);
		//end programm
		static bool shouldShutdown();
		static void setShutdown(bool value);
};


void setNonBlocking(int fd);
void handleSignInt(int sign);

void printcurrentevent(struct epoll_event *current_events, int ndfs);

#endif
