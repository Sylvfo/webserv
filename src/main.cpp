#include "webserv.hpp"

//http://127.0.0.1:80
int main(int ac, char **av)
{
	WebServ *WeServ;
	//allouer ici ??

	std::cout << "Enter webserv" << std::endl;
	// check args
	try
	{
		WeServ.parsConfigFile(ac, av);
		WeServ.startServers();//(epoll starting);

		while(1)
		{
			WeServ.epollWaiting();
		}
	}
	catch
	{

	}
	WeServ.free();
	
}