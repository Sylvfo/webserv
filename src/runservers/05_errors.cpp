#include "Webserv.hpp"

/*
1 create map with bertrands hardcode
2 check
*/

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
		// ERRS a modifier
		StatusCode = 404;
		return ("www/errors/404.html");//default??
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

// ERRS
void HttpRequest::DefaultErrorPage()
{
	std::cout << SOFT_RED "[DEFAULT_ERROR] Loading default 404 error page" << RESET << std::endl;
	StatusCode = 404;
	fd_Ressource = open("www/errors/default.html" , O_RDONLY);//default file??
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

void HttpRequest::NoErrorPage()//+parametres numero et texte. switch
{
	std::cout << SOFT_RED "[NO_ERROR_PAGE] Generating inline error HTML" << RESET << std::endl;
	AnswerBody.clear();
	std::cout << SOFT_RED "[NO_ERROR_PAGE] Inline error HTML generated" << RESET << std::endl;
	// here use 
	std::map<int, std::string>::iterator it = this->Server->default_error_html.find(StatusCode);
	if (it == Server->default_error_html.end())
	{
		// ERRS a revoir
		AnswerBody = "<html><head><title>403 Forbidden</title></head><body><center><h1>403 Forbidden</h1></center><hr><center>Webserv</center>";
	}
	else 
		AnswerBody = it->second;
}
