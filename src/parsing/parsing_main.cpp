#include "webserv.hpp"

WebServ *parseConfigFile(int ac; char **av)
{
	Webserv *general;

	if (ac == 1)
		// general = file par defaut
	else
	{
		if (checkArgs(ac, av) == false)
			throw;
		// general = parse(av[1]);
	}

	return general;
}


WebServ *defaultConfig()

WebServ	*FileConfig()