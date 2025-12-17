#include "Webserv.hpp"

void HttpRequest::AnswerError()
{
	std::string errorPath = GetCustomErrorPage();
	
	if (!errorPath.empty())
	{
		// Page custom trouvée, tente de l'ouvrir
		SetContentType(errorPath);
		fd_Ressource = open(errorPath.c_str(), O_RDONLY);
		if (fd_Ressource >= 0)
		{
			loadRessource();
		}
		else
		{
			UseDefaultErrorHTML();
		}
	}
	else
	{
		UseDefaultErrorHTML();
	}
	SetStatusLine();
	SetResponseHeader();
}

std::string HttpRequest::GetCustomErrorPage()
{
	std::map<int, std::string>::iterator it = this->Server->error_pages.find(StatusCode);
	if (it == Server->error_pages.end())
	{
		return ("");
	}
	std::string path = "www/" + it->second;
	return (path);
}

void HttpRequest::UseDefaultErrorHTML()
{
	AnswerBody.clear();
	
	std::map<int, std::string>::iterator it = this->Server->default_error_html.find(StatusCode);
	AnswerBody = it->second;
	
	ContentType = "text/html";
	ContentLength = AnswerBody.size();
}
