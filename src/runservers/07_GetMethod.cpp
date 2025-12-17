#include "Webserv.hpp"

void HttpRequest::GetRequest()
{
	if (GetAccessRessource() == true)
	{
		if (loadRessource() == true)
		{
			StatusCode = 200;
		}
	}
	else
	{
		AnswerType = ERROR;
	}
}

bool HttpRequest::GetAccessRessource()
{
	std::string makingPath = this->Path;
	std::string decodedUri = urlDecode(this->uri);
	
	LocationConfig* matchingLocation = NULL;
	size_t bestMatchLength = 0;
	
	for (size_t i = 0; i < Server->locations.size(); ++i)
	{
		const std::string& locationPath = Server->locations[i].path;
		if (decodedUri.find(locationPath) == 0 && locationPath.length() > bestMatchLength)
		{
			matchingLocation = &Server->locations[i];
			bestMatchLength = locationPath.length();
		}
	}
	
	// Check if path is a directory
	struct stat pathStat;
	if (stat(makingPath.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode))
	{
		// If URI doesn't end with '/', redirect to add it
		if (!decodedUri.empty() && decodedUri[decodedUri.length() - 1] != '/')
		{
			StatusCode = 301; // Moved Permanently
			// The redirect will be handled by the response builder
			return false;
		}
		
		std::string index = matchingLocation ? matchingLocation->index : "index.html";
		if (index.empty())
			index = "index.html";
		
		// Append index file to directory path
		if (makingPath[makingPath.length() - 1] != '/')
			makingPath += "/";
		makingPath += index;
		
		// Check if index file exists in directory
		if (access(makingPath.c_str(), F_OK) != 0)
		{
			StatusCode = 403;
			AnswerType = ERROR;
			return false;
		}
	}
	const char *path = makingPath.c_str();
	
	// Check if file exists first
	if (access(path, F_OK) != 0)
	{
		StatusCode = 404;
		AnswerType = ERROR;
		return (false);
	}
	
	// Check if we have read permission
	if (access(path, R_OK) != 0)
	{
		StatusCode = 403;
		AnswerType = ERROR;
		return (false);
	}
	
	fd_Ressource = open(path , O_RDONLY);
	if (fd_Ressource < 0)
	{
		StatusCode = 500;
		AnswerType = ERROR;
		return (false);
	}
	SetContentType(makingPath);
	return true;
}

bool HttpRequest::loadRessource()
{
	AnswerBody.clear();
	char buff[4096];
	ssize_t bytesRead;

	if (fd_Ressource == -1)
	{
		UseDefaultErrorHTML();
		return true;
	}
	
	try {
		// Get file size and reserve memory to avoid reallocations
		off_t fileSize = lseek(fd_Ressource, 0, SEEK_END);
		lseek(fd_Ressource, 0, SEEK_SET); // Reset to beginning
		
		if (fileSize > 0)
		{
			AnswerBody.reserve(fileSize);
		}
		
		while ((bytesRead = read(fd_Ressource, buff, sizeof(buff))) > 0)
		{
			AnswerBody.append(buff, bytesRead);
		}
		//check eof
		close(fd_Ressource);
		ContentLength = AnswerBody.size();
		return true;
		
	} catch (const std::bad_alloc& e) {
		close(fd_Ressource);
		StatusCode = 500;
		AnswerBody = "Internal Server Error: File too large";
		ContentLength = AnswerBody.size();
		return false;
	} catch (const std::exception& e) {
		close(fd_Ressource);
		StatusCode = 500;
		return false;
	}
}

void HttpRequest::SetContentType(std::string &makingPath)
{
	std::string Extension;
	size_t dot = makingPath.find_last_of('.');
	if (dot == std::string::npos)
	{
		Extension = "defaut";
	}
	else
	{
		Extension = makingPath.substr(dot);
	}
	std::map<std::string, std::string>::iterator it = Server->mime_types.find(Extension);
	if (it == Server->mime_types.end())
	{
		ContentType = "application/octet-stream";
	}
	else
	{
		try {
			ContentType = it->second;
		} catch (const std::bad_alloc& e) {
			ContentType = "application/octet-stream";
		}
	}
}

