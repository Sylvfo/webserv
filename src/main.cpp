#include "Webserv.hpp"

//signal handler return NULL au debut du programme

//http://127.0.0.1:80
int main(int ac, char **av)
{
	WebServ *WeServ = NULL; //allouer ici ??
	signal(SIGINT, handleSignInt);
	std::cout << "Enter webserv" << std::endl;

	WeServ = new WebServ;
	// check args
	try
	{
		if (ac == 2)
		{
			WeServ->parseConfig(av[1]);
			//WeServ->printConfig();
		}
		WeServ->startServers();//(epoll starting);
		WeServ->printfds();
		while(WeServ->epollWaiting() == true) //listening
			;
	}
	catch (int errCode)
	{
		std::cout << "Error code: " << errCode << std::endl;
		//send response with "500"; Internal Server Error???
	}
	std::cout <<  DARK_PURPLE "Ending: " RESET << std::endl;
	//WeServ->free_webserv();
	delete WeServ;
}

/*
//	Main to test parsing only
int main(int ac, char **av)
{
	WebServ webserv;
	if (ac == 2)
	{
		try
		{
			webserv.parseConfig(av[1]);
			webserv.printConfig();
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error: " << e.what() << std::endl;
			return 1;
		}
		return 0;
	}
	else
	{
		std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
		return 1;
	}
}*/