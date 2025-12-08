#include "Webserv.hpp"

void HttpRequest::AnswerError()
{
	std::cout << "Errors " << std::endl;
	initMimeTypes();// a deplacer
	std::string errorPath;
	//find wich error

	int ErrorCode = atoi(StatusCode.substr(3).c_str());//stati???
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

	//std::cout << "fd_Ressource " << fd_Ressource << std::endl;
	if (fd_Ressource == -1)
	{
		throw std::runtime_error("No html error page found");
	}	
	//SetContentType(makingPath);
	mimeTypes[".html"] = "text/html";
	loadRessource();
	SetStatusLine();
	SetResponseHeader();
}

