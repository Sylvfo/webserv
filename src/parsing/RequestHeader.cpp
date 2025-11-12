#include "RequestHeader.hpp"	
#include <sstream>
#include <iostream>
#include <map>

RequestHeader::RequestHeader()
{
}

RequestHeader::~RequestHeader()
{
}

// CPP98 style for compatibility
void RequestHeader::parseRequest(const std::string& request)
{
	// reset
	method.clear();
	uri.clear();
	version.clear();
	headers.clear();

	// isolate header section (stop at first empty line)
	std::string head;
	size_t pos = request.find("\r\n\r\n");
	if (pos != std::string::npos)
		head = request.substr(0, pos);
	else
	{
		pos = request.find("\n\n");
		if (pos != std::string::npos)
			head = request.substr(0, pos);
		else
			head = request;
	}

	std::istringstream stream(head);
	std::string line;

	// parse request line: METHOD SP URI SP VERSION
	if (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		std::istringstream rl(line);
		rl >> method >> uri >> version;
	}

	// parse headers: Key: value
	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty()) break;
		size_t colon = line.find(':');
		if (colon == std::string::npos) continue;
		std::string key = line.substr(0, colon);
		std::string val = line.substr(colon + 1);

		// trim key right
		while (!key.empty() && (key[key.size() - 1] == ' ' || key[key.size() - 1] == '\t'))
			key.erase(key.size() - 1);

		// trim value left
		size_t start = 0;
		while (start < val.size() && (val[start] == ' ' || val[start] == '\t')) ++start;
		if (start > 0) val = val.substr(start);

		// trim value right
		while (!val.empty() && (val[val.size() - 1] == ' ' || val[val.size() - 1] == '\t'))
			val.erase(val.size() - 1);

		headers[key] = val;
	}
}

std::map<std::string, std::string> RequestHeader::getHeaders() const
{
	return headers;
}

// CPP98 style for compatibility
void RequestHeader::printHeaders() const
{
	std::cout << "Method: " << method << std::endl;
	std::cout << "URI: " << uri << std::endl;
	std::cout << "Version: " << version << std::endl;
	std::cout << "Headers:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		std::cout << it->first << ": " << it->second << std::endl;
	}
}