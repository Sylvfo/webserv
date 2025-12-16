#include "Webserv.hpp"

void HttpRequest::AnswerError()
{
	std::cout << SOFT_RED "[ANSWER_ERROR] Generating error response for status code " << StatusCode << RESET << std::endl;
	std::string errorPath = GetCustomErrorPage();
	
	if (!errorPath.empty())
	{
		// Page custom trouvée, tente de l'ouvrir
		std::cout << SOFT_RED "[ANSWER_ERROR] Using custom error page: " << errorPath << RESET << std::endl;
		SetContentType(errorPath);
		fd_Ressource = open(errorPath.c_str(), O_RDONLY);
		
		if (fd_Ressource >= 0)
		{
			std::cout << SOFT_RED "[ANSWER_ERROR] Custom error page opened successfully (fd: " << fd_Ressource << ")" << RESET << std::endl;
			loadRessource();
		}
		else
		{
			std::cout << SOFT_RED "[ANSWER_ERROR] Failed to open custom error page, using default HTML" << RESET << std::endl;
			UseDefaultErrorHTML();
		}
	}
	else
	{
		// Pas de page custom, utiliser la page par défaut de la map
		std::cout << SOFT_RED "[ANSWER_ERROR] No custom error page, using default HTML from map" << RESET << std::endl;
		UseDefaultErrorHTML();
	}
	
	std::cout << SOFT_RED "[ANSWER_ERROR] Setting status line..." << RESET << std::endl;
	SetStatusLine();
	std::cout << SOFT_RED "[ANSWER_ERROR] Setting response headers..." << RESET << std::endl;
	SetResponseHeader();
	std::cout << SOFT_RED "[ANSWER_ERROR] Error response complete" << RESET << std::endl;
}

std::string HttpRequest::GetCustomErrorPage()
{
	std::cout << SOFT_RED "[GET_CUSTOM_ERROR] Looking for custom error page for code " << StatusCode << RESET << std::endl;
	std::map<int, std::string>::iterator it = this->Server->error_pages.find(StatusCode);
	if (it == Server->error_pages.end())
	{
		std::cout << SOFT_RED "[GET_CUSTOM_ERROR] No custom error page configured" << RESET << std::endl;
		return ("");
	}
	std::string path = "www/" + it->second;
	std::cout << SOFT_RED "[GET_CUSTOM_ERROR] Found custom error page: " << path << RESET << std::endl;
	return (path);
}

void HttpRequest::UseDefaultErrorHTML()
{
	std::cout << SOFT_RED "[USE_DEFAULT_HTML] Loading default HTML for code " << StatusCode << RESET << std::endl;
	AnswerBody.clear();
	
	std::map<int, std::string>::iterator it = this->Server->default_error_html.find(StatusCode);
	AnswerBody = it->second;
	std::cout << SOFT_RED "[USE_DEFAULT_HTML] Using HTML from default_error_html map" << RESET << std::endl;
	
	ContentType = "text/html";
	ContentLength = AnswerBody.size();
	std::cout << SOFT_RED "[USE_DEFAULT_HTML] Content-Length: " << ContentLength << " bytes" << RESET << std::endl;
}
