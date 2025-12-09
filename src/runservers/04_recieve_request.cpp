#include "Webserv.hpp"

//some checks to do
bool HttpRequest::recieveRequest()
{
		//	if (std::cin.eof())
	//what mistake tye can happen?
	// how to do it non blocking
	int BUFFER_SIZE = 30720;//??
	char buff[30720] = {0};
	int bytes = recv(socket_fd, buff, BUFFER_SIZE, 0);
	if (bytes < 0)
		return false;
	RawRequest = buff;
	return true;
}

void HttpRequest::parseRequest()
{
	HTTPHeader.parseRequest(RawRequest);
	HTTPHeader.parseHeaderRequest();
	// extract body for POST requests
	extractRequestBody();
}

void HttpRequest::checkRequest()
{
	AnswerType = STATIC;
	
	LocationConfig* matchingLocation = NULL;
	size_t bestMatchLength = 0;
	
	// Find the best matching location (longest prefix match)
	for (size_t i = 0; i < Server->locations.size(); ++i)
	{
		const std::string& locationPath = Server->locations[i].path;
		if (uri.find(locationPath) == 0 && locationPath.length() > bestMatchLength)
		{
			matchingLocation = &Server->locations[i];
			bestMatchLength = locationPath.length();
		}
	}
	// if the location is a CGI set it to 'CGI'
	if (matchingLocation)
	{
		CGIHandler cgiHandler;
		if (cgiHandler.isCGI(uri, *matchingLocation))
		{
			AnswerType = ::CGI;
		}
	}
}

void HttpRequest::printHttpRequest()
{
	std::cout << "printHttpRequest \n" <<
	"method " << method << " uri " << uri << std::endl;
}

void HttpRequest::extractRequestBody()
{
	//body is after header
	//should be seperated by: \r\n\r\n
	size_t pos = RawRequest.find("\r\n\r\n");

	if (pos != std::string::npos)
	{
		// found it
		// is 4 character so this gets offset
		RequestBody = RawRequest.substr(pos + 4);
	}
	else
	{
		// if client uses other Formatting (\n\n)
		pos = RawRequest.find("\n\n");
		if (pos != std::string::npos)
		{
			// now offset is only 2
			RequestBody = RawRequest.substr(pos + 2);
		}
		// if nothig is found, body stays empty like in GET requests
	}

	// Debug
	if (!RequestBody.empty())
		std::cout << "Exracted body (" << RequestBody.size() << " bytes): "
			<< RequestBody << std::endl;
}
