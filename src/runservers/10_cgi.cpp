#include "Webserv.hpp"

void HttpRequest::AnswerCGI()
{
	std::cout << BRIGHT_MAGENTA "[CGI] Starting CGI execution for URI: " << uri << RESET << std::endl;
	LocationConfig* matchingLocation = NULL;
	size_t bestMatchLength = 0;

	std::cout << BRIGHT_MAGENTA "[CGI] Searching for matching location..." << RESET << std::endl;
	for (size_t i = 0; i < Server->locations.size(); ++i)
	{
		const std::string& locationPath = Server->locations[i].path;
		if (uri.find(locationPath) == 0 && locationPath.length() > bestMatchLength)
		{
			matchingLocation = &Server->locations[i];
			bestMatchLength = locationPath.length();
			std::cout << BRIGHT_MAGENTA "[CGI] Found match: " << locationPath << " (length: " << locationPath.length() << ")" << RESET << std::endl;
		}
	}
	
	if (!matchingLocation)
	{
		std::cout << SOFT_RED "[CGI] No matching location found" << RESET << std::endl;
		HttpAnswer = "HTTP/1.1 404 Not Found\r\n\r\nLocation not found";
		return;
	}
	std::cout << BRIGHT_MAGENTA "[CGI] Best match: " << matchingLocation->path << RESET << std::endl;

	CGIHandler cgiHandler;
	std::string cleanUri = uri;
	size_t queryPos = cleanUri.find('?');
	if (queryPos != std::string::npos)
	{
		std::cout << BRIGHT_MAGENTA "[CGI] Removing query string from URI" << RESET << std::endl;
		cleanUri = cleanUri.substr(0, queryPos);
	}
	std::cout << BRIGHT_MAGENTA "[CGI] Clean URI: " << cleanUri << RESET << std::endl;
	
	std::string scriptPath;
	if (!matchingLocation->root.empty())
	{
		std::cout << BRIGHT_MAGENTA "[CGI] Using location root: " << matchingLocation->root << RESET << std::endl;
		scriptPath = matchingLocation->root;
		if (scriptPath[scriptPath.length() - 1] != '/')
			scriptPath += "/";

		std::string relativePath = cleanUri.substr(matchingLocation->path.length());
		if (!relativePath.empty() && relativePath[0] == '/')
			relativePath = relativePath.substr(1);
		scriptPath += relativePath;
	}
	else
	{
		std::cout << BRIGHT_MAGENTA "[CGI] Using server root: " << Server->root << RESET << std::endl;
		scriptPath = Server->root;
		if (scriptPath[scriptPath.length() - 1] != '/')
			scriptPath += "/";
		scriptPath += cleanUri.substr(1);
	}	

	std::cout << BRIGHT_MAGENTA "[CGI] Script path: " << scriptPath << RESET << std::endl;
	std::cout << BRIGHT_MAGENTA "[CGI] Executing CGI script..." << RESET << std::endl;
	HttpAnswer = cgiHandler.executeCGI(scriptPath, *this, *matchingLocation, RawBody);
	std::cout << BRIGHT_MAGENTA "[CGI] CGI execution complete" << RESET << std::endl;
}