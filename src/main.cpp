#include "Webserv.hpp"

//signal handler return NULL au debut du programme

//http://127.0.0.1:80
/*int main(int ac, char **av)
{
	WebServ *WeServ = NULL; //allouer ici ??
	signal(SIGINT, handleSignInt);
	std::cout << "Enter webserv" << std::endl;
	// check args

	try
	{
		WeServ->startServers();//(epoll starting);
		while(WeServ->epollWaiting() == true) //listening
			;
	}
	catch (int errCode)
	{
		std::cout << "Error code: " << errCode << std::endl;
	}
	WeServ->free_webserv();
}*/


//	Main function with CGI support
int main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
		return 1;
	}

	WebServ webserv;
	try
	{
		// Parse configuration
		webserv.parseConfig(av[1]);
		webserv.printConfig();
		
		std::cout << "\n🚀 Starting webserv with CGI support..." << std::endl;
		
		// Start servers and begin listening
		webserv.startServers();
		while (webserv.epollWaiting() == true)
			;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	catch (int errCode)
	{
		std::cout << "Error code: " << errCode << std::endl;
		return 1;
	}
	
	return 0;
}