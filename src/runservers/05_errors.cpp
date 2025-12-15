#include "Webserv.hpp"

void HttpRequest::AnswerError()
{
	std::cout << SOFT_RED "[ANSWER_ERROR] Generating error response for status code " << StatusCode << RESET << std::endl;
	std::string errorPath = ErrorCodeInServer();
	std::cout << SOFT_RED "[ANSWER_ERROR] Error page path: " << errorPath << RESET << std::endl;
	SetContentType(errorPath); //mime...
	std::cout << SOFT_RED "[ANSWER_ERROR] Opening error file..." << RESET << std::endl;
	OpenErrorFile(errorPath.c_str());
	std::cout << SOFT_RED "[ANSWER_ERROR] Loading error resource..." << RESET << std::endl;
	loadRessource();
	std::cout << SOFT_RED "[ANSWER_ERROR] Setting status line..." << RESET << std::endl;
	SetStatusLine();
	std::cout << SOFT_RED "[ANSWER_ERROR] Setting response headers..." << RESET << std::endl;
	SetResponseHeader();
	std::cout << SOFT_RED "[ANSWER_ERROR] Error response complete" << RESET << std::endl;
}

std::string HttpRequest::ErrorCodeInServer()
{
	std::cout << SOFT_RED "[ERROR_CODE] Looking for error page for code " << StatusCode << RESET << std::endl;
	std::map<int, std::string>::iterator it = this->Server->error_pages.find(StatusCode);
	if (it == Server->error_pages.end())
	{
		std::cout << SOFT_RED "[ERROR_CODE] No custom error page found, using default 404" << RESET << std::endl;
		StatusCode = 404;
		return ("www/errors/404.html");
	}
	std::cout << SOFT_RED "[ERROR_CODE] Found custom error page: " << it->second << RESET << std::endl;
	return ("www/" + it->second);
}

void HttpRequest::OpenErrorFile(const char *path)
{
	std::cout << SOFT_RED "[OPEN_ERROR] Opening error file: " << path << RESET << std::endl;
	fd_Ressource = open(path , O_RDONLY);
	if (fd_Ressource < 0)
	{
		std::cout << SOFT_RED "[OPEN_ERROR] Failed to open error file, using default" << RESET << std::endl;
		DefaultErrorPage();
	}
	else
	{
		std::cout << SOFT_RED "[OPEN_ERROR] Error file opened with fd: " << fd_Ressource << RESET << std::endl;
	}
}



void HttpRequest::DefaultErrorPage()
{
	std::cout << SOFT_RED "[DEFAULT_ERROR] Loading default 404 error page" << RESET << std::endl;
	StatusCode = 404;
	fd_Ressource = open("www/errors/404.html" , O_RDONLY);//default file??
	if (fd_Ressource < 0)
	{
		std::cout << SOFT_RED "[DEFAULT_ERROR] Failed to open default error page" << RESET << std::endl;
		fd_Ressource = -1;
	}
	else
	{
		std::cout << SOFT_RED "[DEFAULT_ERROR] Default error page opened with fd: " << fd_Ressource << RESET << std::endl;
	}
}

void HttpRequest::NoErrorPage()
{
	std::cout << SOFT_RED "[NO_ERROR_PAGE] Generating inline error HTML" << RESET << std::endl;
	AnswerBody.clear();
	AnswerBody = "<html><body><h1>404 Not Found</h1><p>Custom error page.</p></body></html>";
	std::cout << SOFT_RED "[NO_ERROR_PAGE] Inline error HTML generated" << RESET << std::endl;
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