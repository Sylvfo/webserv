#include "Webserv.hpp"

void HttpRequest::Answerlocal()
{
	HttpRequest::CreateHttpAnswerHead();
//	HTTPAnswer = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
	//AnswerBody = "<html><h1>hoi hoi hoi babe</h1></html>";
	///ici open c'est sur le root
	AnswerBody = "";
	std::cout << PASTEL_AQUA "Enter Answer " << RESET << std::endl;

	//if (HTTPHeader.getAccept() == TXT)
//	{
		if ( HTTPHeader.getUri() == "/" || HTTPHeader.getUri() == "")
			loadIndexPage();
		else
			loadPage();
//	}
	
/*	{
		int fd_file;
		fd_file = open( "www/origameee/byChance.html" , 'r');
		char buff;
		while (read(fd_file, &buff, 1 ) > 0)
		{
			AnswerBody += buff;
		}
		//check eof
		close(fd_file);
	//	std::cout << "AnswerBody " << AnswerBody << std::endl;
		HTTPAnswer.append(IntToString(AnswerBody.size()));
		HTTPAnswer.append("\n\n");
		HTTPAnswer.append(AnswerBody);
	}*/
		//	findPage();

}

/*
answer: status line

//find the file...

uri/ url

	if 

*/


void HttpRequest::loadIndexPage()
{
	
	std::string makingPath = Server.root + Server.locations[0].index;
	const char *pathi = "www/origameee/index.html";


	std::cout << LIGHT_CYAN "PATH " << pathi << RESET << std::endl;
	int fd_file;
	fd_file = open(pathi , 'r');
	char buff;
	while (read(fd_file, &buff, 1 ) > 0)
	{
		AnswerBody += buff;
	}
	//check eof
	close(fd_file);
//	std::cout << "AnswerBody " << AnswerBody << std::endl;
	HTTPAnswer.append(IntToString(AnswerBody.size()));
	HTTPAnswer.append("\n\n");
	HTTPAnswer.append(AnswerBody);

}

void HttpRequest::loadPage()
{
	//const char *path = HTTPHeader.getUri().c_str();
	std::string makingPath = Server.root + HTTPHeader.getUri();
//	std::string makingPath = "www/origameee" + HTTPHeader.getUri();
//	const char *path = HTTPHeader.getPath().c_str();
	const char *path = makingPath.c_str();
	std::cout << LIGHT_ROSE "PATH " << path << RESET << std::endl;
	int fd_file;
	fd_file = open(path , 'r');
	char buff;
	while (read(fd_file, &buff, 1 ) > 0)
	{
		AnswerBody += buff;
	}
	//check eof
	close(fd_file);
//	std::cout << "AnswerBody " << AnswerBody << std::endl;
	HTTPAnswer.append(IntToString(AnswerBody.size()));
	HTTPAnswer.append("\n\n");
	HTTPAnswer.append(AnswerBody);
}

void HttpRequest::CreateHttpAnswerHead()
{
//	HTTPAnswer = "HTTP/1.0 200 OK\nContent-Type: text/html\nContent-Length: ";
	HTTPAnswer = "HTTP/1.0 " + IntToString(200) + " OK\nContent-Type: " + HTTPHeader.getAccept() + "\nContent-Length: ";
	std::cout << "HTTPAnswer " << HTTPAnswer << std::endl;
}