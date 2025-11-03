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

#endif