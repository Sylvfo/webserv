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


void WebServ::handleRequest(int indexServ, int connexion_fd)
{
	HttpRequest	Request;
	ServerConfig thisServer;
	char buf[4096];
	int n;
	(void) indexServ;

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

	//Store request head

	while ((n = recv(connexion_fd, buf, sizeof(buf), 0)) > 0)
		Request.RequestHead.append(buf, buf + n);
	
	if (n < -1)
	{
		std::cout << "Error receiving request body" << std::endl;
		return;
	}
	if (Request.RequestHead.empty())
	{
		std::cout << "Empty request body" << std::endl;
		return;
	}

	thisServer = getServer(getRequestHost(Request.RequestHead));
	std::cout << "Request received on server: " << thisServer.server_name << " on port " << thisServer.listen_port << std::endl;
	std::cout << thisServer.root << std::endl;
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


