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
#include <cstring>
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

#define MAX_EVENTS 300

struct ConnectionData
{
    int client_fd;
	int server_fd;
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

		/* Parsing functions */
		void defaultConfig();
		void FileConfig();
		void addServer(ServerConfig config);
		std::vector<ServerConfig> getServerList();
		void parseConfig(std::string path);		
		void printConfig();						

		/* Init server */
		void initServers();
		int initServerSocket(ServerConfig &server, int index);
		bool checkExistingPort(int index);
		void initPoll();
		void startServers();
		void initErroCode(ServerConfig &server);
		void initMimeTypes(ServerConfig &server);
		void initDefautlPage(ServerConfig &server);

		/* Epoll functions */
		bool	epollWaiting();
		int		newConnection(epoll_event new_event);
		ConnectionData* CreateConnection(int index, int new_socket);
		bool	acceptConnection(int index);
		void	closeConnection(epoll_event current_event);

		/* Handle request */
		void handleRequest(epoll_event current_event);

		/* Exit functions*/
		static bool shouldShutdown();
		static void setShutdown(bool value);
};

void setNonBlocking(int fd);
void handleSignInt(int sign);

#endif
