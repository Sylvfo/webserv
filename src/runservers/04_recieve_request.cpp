#include "Webserv.hpp"

//some checks to do
void HttpRequest::recieveRequest() 
{
		//	if (std::cin.eof())
	//what mistake tye can happen?
	// how to do it non blocking
	int BUFFER_SIZE = 30720;//??
	char buff[30720] = {0};
	int bytes = recv(socket_fd, buff, BUFFER_SIZE, 0);
//	if (bytes < 0)
//		throw 11;
	(void) bytes;
	RawRequest = buff;
	
}
	
void HttpRequest::parseRequest()
{
	HTTPHeader.parseRequest(RawRequest);
	HTTPHeader.parseHeaderRequest();
	HTTPHeader.printHeaders();
	//à changer
	//link with the socket and the connexion and the server here?????
}

void HttpRequest::checkRequest()
{
	AnswerType = LOCAL;
	//std::cout << "Request received on server: " << thisServer.server_name << " on port " << thisServer.listen_port << std::endl;
	// to do 
	// answer type??
	//check request avec port et nom de domaine
	// check if data exist and are accessible
}

void HttpRequest::printHttpRequest()
{
	std::cout << "printHttpRequest \n" <<
	"method " << method << " uri " << uri << std::endl;
}