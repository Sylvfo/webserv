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
	}
}

bool HttpRequest::GetAccessRessource()
{
	std::string makingPath;
	// Decode the URI to handle spaces and special characters in filenames
	std::string decodedUri = urlDecode(this->uri);
	
	// Find the best matching location
	LocationConfig* matchingLocation = NULL;
	size_t bestMatchLength = 0;
	
	for (size_t i = 0; i < Server->locations.size(); ++i)
	{
		const std::string& locationPath = Server->locations[i].path;
		if (decodedUri.find(locationPath) == 0 && locationPath.length() > bestMatchLength)
		{
			matchingLocation = &Server->locations[i];
			bestMatchLength = locationPath.length();
			std::cout << SOFT_GREEN "[GET_ACCESS] Found matching location: " << locationPath << RESET << std::endl;
		}
	}
	
	// Determine the root to use
	std::string root;
	if (matchingLocation && !matchingLocation->root.empty())
	{
		root = matchingLocation->root;
		std::cout << SOFT_GREEN "[GET_ACCESS] Using location root: " << root << RESET << std::endl;
	}
	else
	{
		root = Server->root;
		std::cout << SOFT_GREEN "[GET_ACCESS] Using server root: " << root << RESET << std::endl;
	}
	
	// Build the path
	if (decodedUri == "/")
	{
		std::string index = matchingLocation ? matchingLocation->index : Server->locations[0].index;
		if (index.empty())
			index = "index.html";
		makingPath = root + "/" + index;
		std::cout << SOFT_GREEN "[GET_ACCESS] Root URI, using index: " << makingPath << RESET << std::endl;
	}
	else
	{
		if (matchingLocation)
		{
			// Remove location path from URI to get relative path
			std::string relativePath = decodedUri.substr(matchingLocation->path.length());
			if (!relativePath.empty() && relativePath[0] == '/')
				relativePath = relativePath.substr(1);
			
			if (root[root.length() - 1] != '/')
				root += "/";
			makingPath = root + relativePath;
		}
		else
		{
			makingPath = root + decodedUri;
		}
		std::cout << SOFT_GREEN "[GET_ACCESS] Building path: " << makingPath << RESET << std::endl;
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
	}
	const char *path = makingPath.c_str();
	std::cout << SOFT_GREEN "[GET_ACCESS] Opening file: " << path << RESET << std::endl;
	fd_Ressource = open(path , O_RDONLY);
	if (fd_Ressource < 0)
	{
		std::cout << SOFT_RED "[GET_ACCESS] Failed to open file, setting 404" << RESET << std::endl;
		StatusCode = 404;//other error??? 404??
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

