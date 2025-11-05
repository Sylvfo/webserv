#include "Webserv.hpp"

//signal handler return NULL au debut du programme

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
		while(WeServ->epollWaiting() == true) //listening
			;
	}
	catch (int errCode)
	{
		std::cout << "Error code: " << errCode << std::endl;
	}
	WeServ->free_webserv();
	
}