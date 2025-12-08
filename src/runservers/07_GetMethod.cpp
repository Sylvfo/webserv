#include "Webserv.hpp"

void HttpRequest::GetRequest()
{
//	std::cout << PASTEL_AQUA "Enter GET Local Answer " << RESET << std::endl;
	if (GetAccessRessource() == true)
	{
		if (loadRessource() == true)
		{
			StatusCode = 200;//a deplacer
		}
	}
}

bool HttpRequest::GetAccessRessource()
{
	std::string makingPath;
	if (HTTPHeader.getUri() == "/")
		makingPath = Server->root + "/" + Server->locations[0].index;
	else
		makingPath = Server->root + HTTPHeader.getUri();
	const char *path = makingPath.c_str();
	fd_Ressource = open(path , O_RDONLY);
	if (fd_Ressource < 0)
	{
		StatusCode = 404;//other error??? 404??
		return (false);
	}
	SetContentType(makingPath);
	return true;
}

bool HttpRequest::loadRessource()
{
//	DefaultErrorPage();
	AnswerBody.clear();
	//Content-Length
	//what problem??
	char buff[4096];  // Buffer de 4KB
	ssize_t bytesRead;

	if (fd_Ressource == -1)
	{
		NoErrorPage();
		return true;
	}
	while ((bytesRead = read(fd_Ressource, buff, sizeof(buff))) > 0)
    {
        AnswerBody.append(buff, bytesRead);
    }
	//check eof
	close(fd_Ressource);
	ContentLenght = IntToString(AnswerBody.size()); //pas le droit
//	std::cout << "CONTENT LENGTH " << ContentLenght << std::endl;
	return true;
}

void HttpRequest::SetContentType(std::string &makingPath)
{
	std::string Extension;
	size_t dot = makingPath.find_last_of('.');
	if (dot == std::string::npos)
		Extension = "defaut";
	else
		Extension = makingPath.substr(dot);
	std::map<std::string, std::string>::iterator it = Server->mime_types.find(Extension);
	ContentType = it->second;
}

