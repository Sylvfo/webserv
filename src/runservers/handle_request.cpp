#include "Webserv.hpp"
/*
//to test send the answer on an other fd that can be used 

handleRequest(serverConfig &server, current_events[i].data.fd)// server or web serv?? socket id...
{
	HttpRequest	Request;//no need to save it somewhere?

	
	Request.newRequest(server, socket);
	Request.parseRequest();
	Request.checkRequest();
	if (Request.AnswerType == ERROR)
		Request.errortype();
	else if (Request.AnswerType == LOCAL)
		Request.Answerlocal();
	else if (Request.AnswerType == CGI)
		Request.AnswerCGI();
	//check request avec port et nom de domaine ici ou avant?
	//where do we check if we have access to the data?
	//link with the socket and the connexion and the server???
	//how do we recieve the request??
	//
	// parse request (is cgi?)
	// is request valide donc parsing ok + droit down/up load info +info existe?
	// no -> answer error
	// yes and cgi -> send to cgi
	// yes and no cgi -> create answer

	//where are the data sent??? how can we take them for testing?
	//sent on a socket as html or cgi
	Request.sendAnswerToRequest();
}

void HttpRequest::newRequest()
{
	// how to do it non blocking
	int bytes = recv(new_wsocket, buff, BUFFER_SIZE, 0);
	Request = buff;
	//what mistake tye can happen?
}
	
Request.parseRequest()
{
	//put the info in 
}

cgi
{

}

anwerRequest{

}

//downlead???
void sendAnswerToRequest(HttpRequest	&Request)
{
	std::string messagehtml;

	Request.
	//change socket mode... sending....
	//comment couper en morceaux??? par epoll??

	while(totalBytesSent < (int)serverMessage.size())
	{
		//send
		bytesSent = send(new_wsocket, serverMessage.c_str(), serverMessage.size(), 0);
		if (bytesSent < 0)
		{
			std::cout << "Could not send response" ;
		}
		totalBytesSent += bytesSent;
	}
	//change socket mode... ready for new request. 
}

void handleCGI(...)
{
	///!! complicated with env data

	//fork 2 times 1 for timeout the other for ecxcve
	// for 
}*/