#include "Webserv.hpp"

void HttpRequest::AnswerError()
{
	std::cout << "Errors " << std::endl;
	std::string errorPath = ErrorCodeInServer();
	SetContentType(errorPath); //mime...
	OpenErrorFile(errorPath.c_str());
	loadRessource();
	SetStatusLine();
	SetResponseHeader();
}

std::string HttpRequest::ErrorCodeInServer()
{
	std::map<int, std::string>::iterator it = this->Server->error_pages.find(StatusCode);
	if (it == Server->error_pages.end())
	{
		StatusCode = 404;
		return ("www/errors/404.html");
	}
	return ("www/" + it->second);
}

void HttpRequest::OpenErrorFile(const char *path)
{
	fd_Ressource = open(path , O_RDONLY);
	if (fd_Ressource < 0)
		DefaultErrorPage();
}



void HttpRequest::DefaultErrorPage()
{
	StatusCode = 404;
	fd_Ressource = open("www/errors/404.html" , O_RDONLY);//default file??
	if (fd_Ressource < 0)
		fd_Ressource = -1;
}

void HttpRequest::NoErrorPage()
{
	AnswerBody.clear();
	AnswerBody = "<html><body><h1>404 Not Found</h1><p>Custom error page.</p></body></html>";
}
/*
s
check if error is in error pages
no -> default error page

function to make the status line
load page. 
*/ 

/*

switch (ErrorCode)
	{
		case 400:
			fd_Ressource = open("./www/errors/400.html" , O_RDONLY);
			break;
		case 401:
			fd_Ressource = open("./www/errors/401.html" , O_RDONLY);
			break;
		case 403:
			fd_Ressource = open("./www/errors/403.html" , O_RDONLY);
			break;
		case 404:
			fd_Ressource = open("./www/errors/404.html" , O_RDONLY);
			break;
		case 405:
			fd_Ressource = open("./www/errors/405.html" , O_RDONLY);
			break;
		case 500:
			fd_Ressource = open("./www/errors/500.html" , O_RDONLY);
			break;
		case 501:
			fd_Ressource = open("./www/errors/501.html" , O_RDONLY);
			break;
		case 502:
			fd_Ressource = open("./www/errors/502.html" , O_RDONLY);
			break;
		case 503:
			fd_Ressource = open("./www/errors/502.html" , O_RDONLY);
			break;
		default:
			break;
	}
*/