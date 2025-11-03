#include "Webserv.hpp"

//http://127.0.0.1:80
int main(int ac, char **av)
{
	WebServ *WeServ;
	//allouer ici ??
	WeServ = NULL;
	std::cout << "Enter webserv" << std::endl;
	// check args
	try
	{
		WeServ->parseConfigFile(ac, av);
		WeServ->startServers();//(epoll starting);

		while(1)
		{
			WeServ->epollWaiting();
		}
	}
	catch (int errCode)
	{
		std::cout << "Error code: " << errCode << std::endl;
	}
	WeServ->free_webserv();
	
}