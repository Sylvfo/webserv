#include "Webserv.hpp"

void WebServ::startServers()
{
	std::cout << LIGHT_BLUE "[START_SERVERS] Initializing servers..." << RESET << std::endl;
	this->initServers();
	std::cout << LIGHT_BLUE "[START_SERVERS] Initializing epoll..." << RESET << std::endl;
	this->initPoll();
	std::cout << LIGHT_BLUE "[START_SERVERS] All servers initialized successfully" << RESET << std::endl;
}

void WebServ::initServers()
{
	std::cout << LIGHT_BLUE "[INIT_SERVERS] Starting initialization for " << servers.size() << " server(s)" << RESET << std::endl;
	for (size_t i = 0; i < servers.size(); i++)
	{
		std::cout << LIGHT_BLUE "[INIT_SERVERS] Processing server " << i << ": " << servers[i].server_name << ":" << servers[i].listen_port << RESET << std::endl;
		if (checkExistingPort(i) == false)
		{
			std::cout << LIGHT_BLUE "[INIT_SERVERS] Port " << servers[i].listen_port << " is new, creating socket" << RESET << std::endl;
			initServerSocket(servers[i], i);
		}
		else
		{
			std::cout << LIGHT_BLUE "[INIT_SERVERS] Port " << servers[i].listen_port << " already exists, skipping socket creation" << RESET << std::endl;
		}
		std::cout << LIGHT_BLUE "[INIT_SERVERS] Initializing error codes for server " << i << RESET << std::endl;
		initErroCode(servers[i]);// a deplacer apres parsing
		std::cout << LIGHT_BLUE "[INIT_SERVERS] Initializing MIME types for server " << i << RESET << std::endl;
		initMimeTypes(servers[i]);//a deplacer apres parsing
	}
	std::cout << LIGHT_BLUE "[INIT_SERVERS] Server initialization complete" << RESET << std::endl;
}

bool WebServ::checkExistingPort(int index)
{
	std::cout << LIGHT_BLUE "[CHECK_PORT] Checking if port " << servers[index].listen_port << " already exists" << RESET << std::endl;
	for (int k = 0; k < index; k++)
	{
		if (servers[k].listen_port == servers[index].listen_port)
		{
			std::cout << LIGHT_BLUE "[CHECK_PORT] Port " << servers[index].listen_port << " found in server " << k << RESET << std::endl;
			return true;
		}
	}
	std::cout << LIGHT_BLUE "[CHECK_PORT] Port " << servers[index].listen_port << " is unique" << RESET << std::endl;
	return false;
}

//revoir la théorie
int WebServ::initServerSocket(struct ServerConfig &server, int index)
{
	std::cout << LIGHT_BLUE "[INIT_SOCKET] Creating socket for server " << index << RESET << std::endl;
	int fd_socket_servers;

	fd_socket_servers = socket(AF_INET, SOCK_STREAM, 0); // = IPV4, stream, 0 = TCP
	if (fd_socket_servers == -1)
	{
		std::cout << SOFT_RED "[INIT_SOCKET] ERROR: Failed to create socket" << RESET << std::endl;
		throw std::runtime_error("Error in socket server");
	}
	std::cout << LIGHT_BLUE "[INIT_SOCKET] Socket created with fd: " << fd_socket_servers << RESET << std::endl;

	int opt = 1;
	std::cout << LIGHT_BLUE "[INIT_SOCKET] Setting SO_REUSEADDR option" << RESET << std::endl;
	setsockopt(fd_socket_servers, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));//mieux comprendre SO_REUSEPORT??

	//bind on local address
	std::cout << LIGHT_BLUE "[INIT_SOCKET] Binding socket to port " << server.listen_port << RESET << std::endl;
	server.sockaddr.sin_family = AF_INET;//tout le temps pareil
	server.sockaddr.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	server.sockaddr.sin_port = htons(server.listen_port);//
	server.server_len = sizeof(sockaddr);
	if (bind(fd_socket_servers, (struct sockaddr *)&server.sockaddr, server.server_len)!= 0)
	{
		std::cout << SOFT_RED "[INIT_SOCKET] ERROR: bind() failed for port " << server.listen_port << RESET << std::endl;
		throw std::runtime_error("bind() failed");
	}
	std::cout << LIGHT_BLUE "[INIT_SOCKET] Socket bound successfully" << RESET << std::endl;
	
	std::cout << LIGHT_BLUE "[INIT_SOCKET] Setting listen queue to 1024" << RESET << std::endl;
	if (listen(fd_socket_servers, 1024) != 0) //avant ct 20
	{
		std::cout << SOFT_RED "[INIT_SOCKET] ERROR: listen() failed" << RESET << std::endl;
		throw std::runtime_error("listen() failed");
	}
	std::cout << LIGHT_BLUE "[INIT_SOCKET] Setting socket to non-blocking mode" << RESET << std::endl;
	setNonBlocking(fd_socket_servers);
	std::cout << SOFT_GREEN "✓ [INIT_SOCKET] Listening on http://" << server.server_name << ":" << server.listen_port << RESET << std::endl;
	server.fd_socket_serv = fd_socket_servers;//utile oui :)
	std::cout << LIGHT_BLUE "[INIT_SOCKET] Server fd stored: " << server.fd_socket_serv << RESET << std::endl;
	
	std::cout << LIGHT_BLUE "[INIT_SOCKET] Creating ConnectionData for server" << RESET << std::endl;
	ConnectionData* connInfo = new ConnectionData();
	connInfo->server_index = index;
	ServersConnections.insert(std::make_pair(fd_socket_servers, connInfo));
	std::cout << LIGHT_BLUE "[INIT_SOCKET] Server connection added to map" << RESET << std::endl;
	return fd_socket_servers;
}


void WebServ::initPoll()
{
	std::cout << LIGHT_BLUE "[INIT_POLL] Creating epoll instance" << RESET << std::endl;
	this->epollFd = epoll_create(1);//pk zero?? const?? mettre dans
	if (epollFd < 0)
	{
		std::cout << SOFT_RED "[INIT_POLL] ERROR: epoll_create() failed" << RESET << std::endl;
		throw std::runtime_error("epoll_create() failed");
	}
	std::cout << LIGHT_BLUE "[INIT_POLL] Epoll fd created: " << epollFd << RESET << std::endl;
	
	std::cout << LIGHT_BLUE "[INIT_POLL] Adding " << ServersConnections.size() << " server(s) to epoll" << RESET << std::endl;
	std::map<int, ConnectionData*>::iterator it;
	for (it = ServersConnections.begin(); it != ServersConnections.end(); ++it)
	{
		std::cout << LIGHT_BLUE "[INIT_POLL] Adding server fd " << it->first << " to epoll" << RESET << std::endl;
		struct epoll_event event;
		//event.events = EPOLLIN | EPOLLET;
		event.events = EPOLLIN; // EPOLLET would force me to use errno after recv which is not allowed
		it->second->client_fd = 0;
		it->second->server_fd = it->first;
		event.data.ptr = it->second;
		if (epoll_ctl(epollFd, EPOLL_CTL_ADD,it->first, &event) < 0)
		{
			std::cout << SOFT_RED "[INIT_POLL] ERROR: epoll_ctl() failed for server fd " << it->first << RESET << std::endl;
			throw std::runtime_error("epoll_ctl() for servers failed");
		}
		std::cout << LIGHT_BLUE "[INIT_POLL] Server fd " << it->first << " added to epoll successfully" << RESET << std::endl;
	}
	std::cout << LIGHT_BLUE "[INIT_POLL] All servers added to epoll" << RESET << std::endl;
}

void setNonBlocking(int fd)
{
	std::cout << LIGHT_BLUE "[SET_NONBLOCK] Setting fd " << fd << " to non-blocking" << RESET << std::endl;
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags |O_NONBLOCK);
	std::cout << LIGHT_BLUE "[SET_NONBLOCK] Fd " << fd << " set to non-blocking" << RESET << std::endl;
}
