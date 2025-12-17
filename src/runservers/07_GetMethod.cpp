#include "Webserv.hpp"

void HttpRequest::GetRequest()
{
	std::cout << SOFT_GREEN "[GET] Processing GET request for URI: " << uri << RESET << std::endl;
	if (GetAccessRessource() == true)
	{
		std::cout << SOFT_GREEN "[GET] Resource accessible, loading..." << RESET << std::endl;
		if (loadRessource() == true)
		{
			StatusCode = 200;//a deplacer
			std::cout << SOFT_GREEN "[GET] Resource loaded successfully, status 200" << RESET << std::endl;
		}
		else
		{
			std::cout << SOFT_RED "[GET] Failed to load resource" << RESET << std::endl;
		}
	}
	else
	{
		std::cout << SOFT_RED "[GET] Resource not accessible" << RESET << std::endl;
		AnswerType = ERROR;
	}
}

bool HttpRequest::GetAccessRessource()
{
	std::string makingPath = this->Path;
	std::string decodedUri = urlDecode(this->uri);
	
	std::cout << SOFT_GREEN "[GET_ACCESS] Using path from CheckRequest: " << makingPath << RESET << std::endl;
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
		std::cout << SOFT_GREEN "[GET_ACCESS] Path is a directory" << RESET << std::endl;
		
		// If URI doesn't end with '/', redirect to add it
		if (!decodedUri.empty() && decodedUri[decodedUri.length() - 1] != '/')
		{
			std::cout << SOFT_GREEN "[GET_ACCESS] URI missing trailing slash, redirecting" << RESET << std::endl;
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
		std::cout << SOFT_GREEN "[GET_ACCESS] Directory index path: " << makingPath << RESET << std::endl;
		
		// Check if index file exists in directory
		if (access(makingPath.c_str(), F_OK) != 0)
		{
			std::cout << SOFT_RED "[GET_ACCESS] Index file not found in directory, 403 Forbidden" << RESET << std::endl;
			StatusCode = 403;
			AnswerType = ERROR;
			return false;
		}
	}
	const char *path = makingPath.c_str();
	std::cout << SOFT_GREEN "[GET_ACCESS] Opening file: " << path << RESET << std::endl;
	
	// Check if file exists first
	if (access(path, F_OK) != 0)
	{
		std::cout << SOFT_RED "[GET_ACCESS] File not found (404)" << RESET << std::endl;
		StatusCode = 404;
		AnswerType = ERROR;
		return (false);
	}
	
	// Check if we have read permission
	if (access(path, R_OK) != 0)
	{
		std::cout << SOFT_RED "[GET_ACCESS] Permission denied (403)" << RESET << std::endl;
		StatusCode = 403;
		AnswerType = ERROR;
		return (false);
	}
	
	fd_Ressource = open(path , O_RDONLY);
	if (fd_Ressource < 0)
	{
		std::cout << SOFT_RED "[GET_ACCESS] Failed to open file (500)" << RESET << std::endl;
		StatusCode = 500;
		AnswerType = ERROR;
		return (false);
	}
	std::cout << SOFT_GREEN "[GET_ACCESS] File opened with fd: " << fd_Ressource << RESET << std::endl;
	SetContentType(makingPath);
	std::cout << SOFT_GREEN "[GET_ACCESS] Returning from GetAccessRessource" << RESET << std::endl;
	return true;
}

bool HttpRequest::loadRessource()
{
	std::cout << SOFT_GREEN "[LOAD_RESOURCE] Loading resource from fd: " << fd_Ressource << RESET << std::endl;
	AnswerBody.clear();
	char buff[4096];
	ssize_t bytesRead;

	if (fd_Ressource == -1)
	{
		std::cout << SOFT_RED "[LOAD_RESOURCE] Invalid fd, generating inline error page" << RESET << std::endl;
		UseDefaultErrorHTML();
		return true;
	}
	
	try {
		// Get file size and reserve memory to avoid reallocations
		off_t fileSize = lseek(fd_Ressource, 0, SEEK_END);
		lseek(fd_Ressource, 0, SEEK_SET); // Reset to beginning
		
		if (fileSize > 0)
		{
			std::cout << SOFT_GREEN "[LOAD_RESOURCE] File size: " << fileSize << " bytes, reserving memory" << RESET << std::endl;
			AnswerBody.reserve(fileSize);
			std::cout << SOFT_GREEN "[LOAD_RESOURCE] Memory reserved successfully" << RESET << std::endl;
		}
		
		int totalBytes = 0;
		while ((bytesRead = read(fd_Ressource, buff, sizeof(buff))) > 0)
		{
			AnswerBody.append(buff, bytesRead);
			totalBytes += bytesRead;
			std::cout << SOFT_GREEN "[LOAD_RESOURCE] Read " << bytesRead << " bytes (total: " << totalBytes << ")" << RESET << std::endl;
		}
		//check eof
		close(fd_Ressource);
		ContentLength = AnswerBody.size();
		std::cout << SOFT_GREEN "[LOAD_RESOURCE] Resource loaded, Content-Length: " << ContentLength << RESET << std::endl;
		return true;
		
	} catch (const std::bad_alloc& e) {
		std::cout << SOFT_RED "[LOAD_RESOURCE] Memory allocation failed: " << e.what() << RESET << std::endl;
		close(fd_Ressource);
		StatusCode = 500;
		AnswerBody = "Internal Server Error: File too large";
		ContentLength = AnswerBody.size();
		return false;
	} catch (const std::exception& e) {
		std::cout << SOFT_RED "[LOAD_RESOURCE] Exception: " << e.what() << RESET << std::endl;
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
		std::cout << SOFT_GREEN "[SET_CONTENT_TYPE] No extension found, using default" << RESET << std::endl;
	}
	else
	{
		Extension = makingPath.substr(dot);
		std::cout << SOFT_GREEN "[SET_CONTENT_TYPE] Extension: " << Extension << RESET << std::endl;
	}
	
	std::map<std::string, std::string>::iterator it = Server->mime_types.find(Extension);
	if (it == Server->mime_types.end())
	{
		std::cout << SOFT_RED "[SET_CONTENT_TYPE] Extension not found in mime_types, using default" << RESET << std::endl;
		ContentType = "application/octet-stream";
	}
	else
	{
		std::cout << SOFT_GREEN "[SET_CONTENT_TYPE] Found mime type for " << Extension << RESET << std::endl;
		try {
			ContentType = it->second;
			std::cout << SOFT_GREEN "[SET_CONTENT_TYPE] Content-Type: " << ContentType << RESET << std::endl;
		} catch (const std::bad_alloc& e) {
			std::cout << SOFT_RED "[SET_CONTENT_TYPE] Failed to copy mime type string" << RESET << std::endl;
			ContentType = "application/octet-stream";
		}
	}
}

