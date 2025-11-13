#include "Webserv.hpp"

//to test send the answer on an other fd that can be used 

//check request avec port et nom de domaine ici ou aprse?

/*TEMP FUNCTION TO REMOVE WHEN THE REQUEST PARSING IS DONE !!!!*/
std::string getRequestHost(std::string req)
{
    std::string::size_type pos = 0;
	std::string host;
    while (true) {
        std::string::size_type end = req.find('\n', pos);
        if (end == std::string::npos) break;

        std::string line = req.substr(pos, end - pos);
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        if (line.size() >= 5 && line.substr(0, 5) == "Host:") {
			host = line.substr(6);
            break;
        }

        pos = end + 1;
	}
	return host;
}

//main HTTP handling function
void WebServ::handleRequest(int indexServ, int connexion_fd)
{
	HttpRequest	Request;
	ServerConfig thisServer;// to put in the request...
	(void) indexServ;

	Request.setSocketFd(connexion_fd);
	Request.Server = getServer(0);
	Request.linkServer(indexServ);// ZOGZOGISSUE COMMENT ON LIE LE SERVER ICI??? CE SERAIT MIEUX D'AVOIR LE INDEX SERV EST LE MEME QUE LE I SERVER FDS
	Request.recieveRequest();//to do better
	Request.parseRequest(); //to do 
	Request.HTTPHeader.printHeaders();
	Request.checkRequest(); //to do 
//	if (Request.AnswerType == ERROR)
//		Request.errortype();//to do 
//	else if (Request.AnswerType == LOCAL)
	Request.Answerlocal();//to do 
//	else if (Request.AnswerType == CGI)
//		Request.AnswerCGI();//to do
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


