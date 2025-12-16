#include "Webserv.hpp"

//http://127.0.0.1:80
int main(int ac, char **av)
{
	signal(SIGINT, handleSignInt);
	signal(SIGPIPE, SIG_IGN);

	WebServ *WeServ = NULL;
	WeServ = new WebServ;
	std::cout << "Enter webserv" << std::endl;

	try
	{
		if (ac == 2)
			WeServ->parseConfig(av[1]);
		else if (ac == 1)
			WeServ->parseConfig("config/default.conf");
		else
		{
			std::cout << SOFT_RED "[ERROR] Usage: ./webserv [config_file]" << RESET << std::endl;
			delete WeServ;
			return 1;
		}
		
		WeServ->startServers();
		while(WeServ->epollWaiting() == true)// && !WebServ::shouldShutdown()) //listening
			;
	}
/*	catch (int errCode)
	{
		std::cout << "Error code: " << errCode << std::endl;
		//send response with "500"; Internal Server Error???
	}*/
	catch (const std::exception &e)
	{
		std::cerr << SOFT_RED "[ERROR] Exception - " << e.what() << RESET << std::endl;
		std::cout <<  DARK_PURPLE "========== WEBSERV ENDING ==========" RESET << std::endl;
		delete WeServ;
		return 1;
	}
	std::cout <<  DARK_PURPLE "========== WEBSERV ENDING ==========" RESET << std::endl;
	delete WeServ;
	return 0;
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