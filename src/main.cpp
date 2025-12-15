#include "Webserv.hpp"

//http://127.0.0.1:80
int main(int ac, char **av)
{
	WebServ *WeServ = NULL;
	signal(SIGINT, handleSignInt);
	std::cout << SOFT_GREEN "========== WEBSERV STARTING ==========" << RESET << std::endl;

	WeServ = new WebServ;
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
		while(WeServ->epollWaiting() == true)
			;
	}
	catch (int errCode)
	{
		std::cout << SOFT_RED "[ERROR] Exception - Error code: " << errCode << RESET << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << SOFT_RED "[ERROR] Exception - " << e.what() << RESET << std::endl;
	}
	std::cout <<  DARK_PURPLE "========== WEBSERV ENDING ==========" RESET << std::endl;
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