#include "Webserv.hpp"

void HttpRequest::AnswerCGI()
{
	std::cout << "[CGI] Starting CGI execution" << std::endl;
	LocationConfig* matchingLocation = NULL;
	size_t bestMatchLength = 0;

	for (size_t i = 0; i < Server->locations.size(); ++i)
	{
		const std::string& locationPath = Server->locations[i].path;
		if (uri.find(locationPath) == 0 && locationPath.length() > bestMatchLength)
		{
			matchingLocation = &Server->locations[i];
			bestMatchLength = locationPath.length();
		}
	}
	
	if (!matchingLocation)
	{
		HttpAnswer = "HTTP/1.1 404 Not Found\r\n\r\nLocation not found";
		return;
	}

	CGIHandler cgiHandler;
	std::string cleanUri = uri;
	size_t queryPos = cleanUri.find('?');
	if (queryPos != std::string::npos)
	{
		cleanUri = cleanUri.substr(0, queryPos);
	}
	
	std::string scriptPath;
	if (!matchingLocation->root.empty())
	{
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
		scriptPath = Server->root;
		if (scriptPath[scriptPath.length() - 1] != '/')
			scriptPath += "/";
		scriptPath += cleanUri.substr(1);
	}	

	HttpAnswer = cgiHandler.executeCGI(scriptPath, HTTPHeader, *matchingLocation, RequestBody);
}