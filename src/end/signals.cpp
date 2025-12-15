#include "Webserv.hpp"

void handleSignInt(int sign)
{
	(void) sign;
	std::cout << std::endl << SOFT_ORANGE "[SIGNAL] SIGINT received (Ctrl+C), shutting down gracefully..." << RESET << std::endl;
//	return;
}
