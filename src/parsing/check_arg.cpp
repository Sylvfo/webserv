#include "Webserv.hpp"

bool checkArgs(int argc, char **argv)
{
	(void) argv;
	if (argc != 2)
	{
		std::cout << "Usage: ./webserv [configuration file]" << std::endl;
		return false;
	}
	return true;
}