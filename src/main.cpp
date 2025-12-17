#include "Webserv.hpp"

int main(int ac, char **av)
{
	signal(SIGINT, handleSignInt);
	signal(SIGPIPE, SIG_IGN);

	WebServ *WeServ = NULL;
	WeServ = new WebServ;
//	sleep(25);
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
		while(WeServ->epollWaiting() == true)
			;
	}
	catch (const std::exception &e)
	{
		std::cerr << SOFT_RED "[ERROR] Exception - " << e.what() << RESET << std::endl;
		std::cout <<  DARK_PURPLE "========== WEBSERV ENDING ==========" RESET << std::endl;
		delete WeServ;
		return 1;
	}
	std::cout <<  DARK_PURPLE "========== WEBSERV ENDING ==========" RESET << std::endl;
	delete WeServ;
//	sleep(25);
	return 0;
}
