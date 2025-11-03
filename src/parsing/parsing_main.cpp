#include "Webserv.hpp"

WebServ WebServ::parseConfigFile(int ac, char **av)
{
	WebServ general;

	if (ac == 1)
		std::cout << "No file specified, starting default config." << std::endl;
	else
	{
		if (checkArgs(ac, av) == false)
			throw 2;
		// general = parse(av[1]);
	}

	return general;
}


void WebServ::defaultConfig()  
{}

void WebServ::FileConfig()
{}