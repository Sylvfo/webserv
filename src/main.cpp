#include "webserv.hpp"

int main(int argc, char **argv)
{
	if (checkArgs(argc, argv) == false)
		return -1;

	std::cout << "Enter webserv" << std::endl;
	return 0;
}