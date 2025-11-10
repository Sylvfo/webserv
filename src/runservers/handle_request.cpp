#include "Webserv.hpp"

//to test send the answer on an other fd that can be used 

//check request avec port et nom de domaine ici ou aprse?

void WebServ::handleRequest(int indexServ, int connexion_fd)
{
	HttpRequest	Request;
	(void) indexServ;
	//struct ServerConfig *thisServer;//find a better name just a pointer to the server

	Request.setSocketFd(connexion_fd);
//	Request.linkServer(indexServ);// ZOGZOGISSUE COMMENT ON LIE LE SERVER ICI??? CE SERAIT MIEUX D'AVOIR LE INDEX SERV EST LE MEME QUE LE I SERVER FDS
//	Request.recieveRequest();//to do better
////	Request.parseRequest(); //to do 
//	Request.checkRequest(); //to do 
//	if (Request.AnswerType == ERROR)
//		Request.errortype();//to do 
//	else if (Request.AnswerType == LOCAL)
	Request.Answerlocal();//to do 
//	else if (Request.AnswerType == CGI)
//		Request.AnswerCGI();//to do
	
	//check request avec port et nom de domaine ici ou avant?
	//link with the socket and the connexion and the server???

	Request.setSocketFd(connexion_fd);
	Request.sendAnswerToRequest();
}

void HttpRequest::setSocketFd(int fd)
{
	socket_fd = fd;
}

void HttpRequest::linkServer(int index)
{
	(void) index;
	// to do 
}

void HttpRequest::recieveRequest()
{
	// how to do it non blocking
/*	int BUFFER_SIZE = 30720;//??
	char buff[30720] = {0};
	int bytes = recv(socket_fd, buff, BUFFER_SIZE, 0);
	Request = buff;*/
	//what mistake tye can happen?
}
	
void HttpRequest::parseRequest()
{
	// to do 
}

void HttpRequest::checkRequest()
{
	// to do 
}

void HttpRequest::errortype()
{
	// to do 
}

void HttpRequest::Answerlocal()
{
	HTTPAnswer = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
	//AnswerBody = "<html><h1>hoi hoi hoi babe</h1></html>";
	///ici open c'est sur le root
	AnswerBody = "";

	int fd_file;
	fd_file = open( "www/origameee/index.html" , 'r');
	char buff;
	while (read(fd_file, &buff, 1 ) > 0)
	{
		AnswerBody += buff;
	}
	close(fd_file);
	std::cout << "AnswerBody " << AnswerBody << std::endl;
	HTTPAnswer.append(IntToString(AnswerBody.size()));
	HTTPAnswer.append("\n\n");
	HTTPAnswer.append(AnswerBody);

	// to do 
}

void HttpRequest::AnswerCGI()
{
	// to do 
}

//download???
void HttpRequest::sendAnswerToRequest()
{
	int bytesSent = 0;
	int totalBytesSent = 0;

	std::cout << "socket fd " << socket_fd << std::endl;
	// is it non blocking?
	while(totalBytesSent < (int)HTTPAnswer.size())
	{
		bytesSent = send(socket_fd, HTTPAnswer.c_str(), HTTPAnswer.size(), 0);
		if (bytesSent < 0)
		{
			std::cout << "Could not send response";
			return; 
		}
		totalBytesSent += bytesSent;
	}
	//change socket mode... ready for new request.
	//change socket mode... sending....
	//comment couper en morceaux??? par epoll??
}

std::string IntToString(int numb)
{
	std::string value;
	std::stringstream out;

	out << numb;
	value = out.str();
	return (value);

}


