#include "Webserv.hpp"

void HttpRequest::answerError()
{
	std::string errorPath = getCustomErrorPage();
	
	if (!errorPath.empty())
	{
		_setcontent_type(errorPath);
		fd_ressource = open(errorPath.c_str(), O_RDONLY);
		if (fd_ressource >= 0)
		{
			_loadRessource();
		}
		else
		{
			useDefaultErrorHTML();
		}
	}
	else
	{
		useDefaultErrorHTML();
	}
	_setStatusLine();
	_setResponseHeader();
}

std::string HttpRequest::getCustomErrorPage()
{
	std::map<int, std::string>::iterator it = this->Server->error_pages.find(status_code);
	if (it == Server->error_pages.end())
	{
		return ("");
	}
	std::string path = "www/" + it->second;
	return (path);
}

void HttpRequest::useDefaultErrorHTML()
{
	answer_body.clear();
	
	std::map<int, std::string>::iterator it = this->Server->default_error_html.find(status_code);
	answer_body = it->second;
	
	content_type = "text/html";
	content_length = answer_body.size();
}
