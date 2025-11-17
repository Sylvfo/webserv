#include "Webserv.hpp"

// create body + find values Header fieds.
void HttpRequest::GetRequest()
{
	std::cout << PASTEL_AQUA "Enter GET Local Answer " << RESET << std::endl;
	initMimeTypes();
	if (GetAccessRessource() == true)
	{
		if (loadRessource() == true)
		{
			StatusCode = "200 OK";
		}
	}
}

bool HttpRequest::GetAccessRessource()
{
	//create path
	std::string makingPath;
	if (HTTPHeader.getUri() == "/") //root URI
		makingPath = Server.root + "/" + Server.locations[0].index;
	else
		makingPath = Server.root + HTTPHeader.getUri();
//	const char *path = "www/origameee/index.html";
/*	std::cout << LIGHT_CYAN "PATH " << pathi << RESET << std::endl;
		//const char *path = HTTPHeader.getUri().c_str();
	std::string makingPath = Server.root + HTTPHeader.getUri();
//	std::string makingPath = "www/origameee" + HTTPHeader.getUri();
//	const char *path = HTTPHeader.getPath().c_str();*/
	const char *path = makingPath.c_str();
	std::cout << LIGHT_ROSE "PATH " << path << RESET << std::endl;
	
	//file exist
/*	struct stat st;
	if (stat(path, &st) == -1)
	{
		if (errno == ENOENT || errno == ENOTDIR)
        {
			StatusCode = "403 Forbidden";
			return (false);
		}
	}*/
	//permission
/*	if (access(path, 'r') == -1)//change in other methods
	{
		StatusCode = "403 Forbidden";
		return (false);
	}*/
	fd_Ressource = open(path , O_RDONLY);
	if (fd_Ressource < 0)
	{
		StatusCode = "403 Forbidden";
		return (false);
	}
	SetContentType(makingPath);
	return true;
}


bool HttpRequest::loadRessource()
{
	AnswerBody.clear();
	//Content-Length
	//what problem??
	char buff[4096];  // Buffer de 4KB
	ssize_t bytesRead;

	 while ((bytesRead = read(fd_Ressource, buff, sizeof(buff))) > 0)
    {
        AnswerBody.append(buff, bytesRead);
    }
	//check eof
	close(fd_Ressource);
	ContentLenght = IntToString(AnswerBody.size()); //pas le droit
	std::cout << "CONTENT LENGTH " << ContentLenght << std::endl;
	return true;
}

/*
bool HttpRequest::loadRessource()
{
	AnswerBody.clear();
	//Content-Length
	//what problem??
	char buff[4096];  // Buffer de 4KB
	
	while (read(fd_Ressource, &buff, 1 ) > 0)
	{
		AnswerBody += buff;
	}
	//check eof
	close(fd_Ressource);
	int CL = AnswerBody.size();
	ContentLenght = IntToString(AnswerBody.size()); //pas le droit
	std::cout << "CONTENT LENGTH " << ContentLenght << " CL " << CL << std::endl;
	return true;
}*/



void HttpRequest::SetContentType(std::string &makingPath)
{
	std::string Extension;
	size_t dot = makingPath.find_last_of('.');
	if (dot == std::string::npos)
		Extension = "defaut";
	else
		Extension = makingPath.substr(dot);
//	(mimeTypes.count(Extension))
	std::map<std::string, std::string>::iterator it = mimeTypes.find(Extension);
	ContentType = it->second;

	std::cout << BRIGHT_LIME << "content type " << ContentType << "till here" << std::endl;
}

void HttpRequest::initMimeTypes() {
    mimeTypes[".html"] = "text/html";
	mimeTypes[".htm"]  = "text/html";
	mimeTypes[".css"]  = "text/css";
    mimeTypes[".js"]   = "application/javascript";
	mimeTypes[".json"] = "application/json";
    mimeTypes[".png"]  = "image/png";
    mimeTypes[".jpg"]  = "image/jpeg";
    mimeTypes[".jpeg"] = "image/jpeg";
	mimeTypes[".gif"]  = "image/gif";
	mimeTypes[".svg"]  = "image/svg+xml";
	mimeTypes[".ico"]  = "image/x-icon";
	mimeTypes[".txt"]  = "text/plain";
	mimeTypes[".pdf"]  = "application/pdf";
	mimeTypes["defaut"]  = "application/octet-stream";
}