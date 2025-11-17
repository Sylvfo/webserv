#include "Webserv.hpp"

void HttpRequest::Answerlocal()
{
	std::cout << PASTEL_AQUA "Enter local Answer " << RESET << std::endl;

	if (method == "GET")
		GetRequest();
	else if (method == "POST")
		PostRequest();
	else if (method == "DELETE")
		DeleteRequest();

	//error??

	SetStatusLine();
	SetResponseHeader();
}


void HttpRequest::SetResponseHeader()
{
    HttpAnswer += "Content-Length: ";
    HttpAnswer += ContentLenght;
    HttpAnswer += "\r\n";
    HttpAnswer += "Content-Type: ";
    HttpAnswer += ContentType;
    HttpAnswer += "\r\n";
	HttpAnswer += "Connection: close\r\n"; 
    HttpAnswer += "\r\n";  // Séparation headers/body
    HttpAnswer += AnswerBody;
    
/*    // LOGS DE DEBUG
    std::cout << DARK_GREEN "=== FULL RESPONSE ===" << RESET << std::endl;
    std::cout << "Length: " << HttpAnswer.size() << " bytes" << std::endl;
    std::cout << "First 200 chars: " << HttpAnswer.substr(0, 200) << std::endl;
    std::cout << DARK_GREEN "===================" << RESET << std::endl;
    
    // Afficher en hexadécimal pour voir les \r\n
    for (size_t i = 0; i < 100 && i < HttpAnswer.size(); i++) {
        if (HttpAnswer[i] == '\r')
            std::cout << "\\r";
        else if (HttpAnswer[i] == '\n')
            std::cout << "\\n\n";
        else
            std::cout << HttpAnswer[i];
    }
    std::cout << std::endl;*/
}


//nouvelle
void HttpRequest::SetStatusLine()
{
	HttpAnswer.clear();
	HttpAnswer = "HTTP/1.0 "; //put in constructor
	HttpAnswer += StatusCode;
	HttpAnswer += "\r\n";
/*	if (StatusCode == "200")
		HttpAnswer += " OK";
	else
		HttpAnswer += " NOT OK"; //to do, see p27*/
//	HttpAnswer += "\n";
}


/*
void HttpRequest::SetResponseHeader()
{
	
    HttpAnswer += "Content-Length: ";
    HttpAnswer += ContentLenght;
    HttpAnswer += "\r\n";
    HttpAnswer += "Content-Type: ";
    HttpAnswer += ContentType;
    HttpAnswer += "\r\n";
    HttpAnswer += "\r\n";  // Séparation headers/body
    HttpAnswer += AnswerBody;  // Ajouter le corps HTML
    
    std::cout << DARK_GREEN "Full Response:\n" << HttpAnswer << RESET << std::endl;
}*/



/*
void HttpRequest::SetResponseHeader()
{
	HttpAnswer += "Content-Length: ";
	HttpAnswer += ContentLenght;
	HttpAnswer += "\r\n";
	HttpAnswer += "Content-Type: ";
	HttpAnswer += ContentType;
	HttpAnswer += "\r\n";
	//Allow

	//Content-Encoding

	//Content-Length

	//Content-Type: 
	
	//Expires pas besoin?

	//Last-Modified pas besoin?

	//extension-header pas besoin?
	HttpAnswer.append("\r\n\r\n");

	std::cout <<  DARK_GREEN "Response Header " << HttpAnswer << RESET << std::endl;
}
*/


/*
void HttpRequest::Answerlocal()
{

	//methodes

	HttpRequest::CreateHttpAnswerHead();
//	HttpAnswer = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
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
	
	{
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
		HttpAnswer.append(IntToString(AnswerBody.size()));
		HttpAnswer.append("\n\n");
		HttpAnswer.append(AnswerBody);
	}
		//	findPage();

}


//ancienne
void HttpRequest::CreateHttpAnswerHead()
{
//	HttpAnswer = "HTTP/1.0 200 OK\nContent-Type: text/html\nContent-Length: ";
	HttpAnswer = "HTTP/1.0 " + IntToString(200) + " OK\nContent-Type: " + HTTPHeader.getAccept() + "\nContent-Length: ";
	std::cout << "HttpAnswer " << HttpAnswer << std::endl;
}



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
	HttpAnswer.append(IntToString(AnswerBody.size()));
	HttpAnswer.append("\n\n");
	HttpAnswer.append(AnswerBody);

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
	//std::cout << "AnswerBody " << AnswerBody << std::endl;
	HttpAnswer.append(IntToString(AnswerBody.size()));
	HttpAnswer.append("\n\n");
	HttpAnswer.append(AnswerBody);
}

*/