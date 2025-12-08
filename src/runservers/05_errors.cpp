#include "Webserv.hpp"

void HttpRequest::AnswerError()
{
	std::cout << "Errors " << std::endl;
	initMimeTypes();// a deplacer
	std::string errorPath = ErrorCodeInServer();
	SetContentType(errorPath); //mime...
	OpenErrorFile(errorPath.c_str());
	loadRessource();
	SetStatusLine();
	SetResponseHeader();
}

std::string HttpRequest::ErrorCodeInServer()
{
	std::map<int, std::string>::iterator it = this->Server->error_pages.find(StatusCodeI);
	if (it == Server->error_pages.end())
	{
		StatusCodeI = 404;
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

void WebServ::initErroCode(struct ServerConfig &server)
{
	server.error_code_message.insert(std::make_pair(200, "200 OK"));
	server.error_code_message.insert(std::make_pair(400, "400 Bad Request"));
	server.error_code_message.insert(std::make_pair(403, "403 Forbidden"));
	server.error_code_message.insert(std::make_pair(404, "404 Not Found"));
	server.error_code_message.insert(std::make_pair(405, "405 Method Not Allowed"));
	server.error_code_message.insert(std::make_pair(500, "500 Internal Server Error"));
	server.error_code_message.insert(std::make_pair(501, "501 Not Implemented"));
	server.error_code_message.insert(std::make_pair(502, "502 Bad Gateway"));
	server.error_code_message.insert(std::make_pair(503, "503 Service Unavailable"));
}

void HttpRequest::DefaultErrorPage()
{
	StatusCodeI = 404;
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