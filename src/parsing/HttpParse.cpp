#include "HttpRequest.hpp"

bool HttpRequest::ParseHeader()
{
	std::stringstream stream(this->RawHeader);
	std::string line;

	if (!std::getline(stream, line))
	{
		std::cout << SOFT_RED "[PARSE_HEADER] Empty request (400)" << RESET << std::endl;
		this->AnswerType = ERROR;
		this->StatusCode = 400;
		return false;
	}

	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	std::cout << BRIGHT_CYAN "[REQUEST] " << line << RESET << std::endl;

	if (!ParseRequestLine(line))
	{
		this->AnswerType = ERROR;
		return false;
	}

	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (line.empty())
			break;

		if (!ParseOneHeader(line))
		{
			std::cout << SOFT_RED "[ERROR] Failed to parse header line: " << line << RESET << std::endl;
			this->AnswerType = ERROR;
			return false;
		}
	}

	if (this->version == "HTTP/1.1" && (this->headers.find("host") == this->headers.end()))
	{
		std::cout << SOFT_RED "[ERROR] HTTP/1.1 requires Host header (400)" << RESET << std::endl;
		this->AnswerType = ERROR;
		this->StatusCode = 400;
		return false;
	}
	return true;
}

bool HttpRequest::ParseRequestLine(const std::string& line)
{
	std::stringstream stream(line);
	std::string method, path, version, junk;

	stream >> method >> path >> version;
	if (method.empty() || path.empty() || version.empty() || (stream >> junk))
	{
		std::cout << SOFT_RED "[ERROR] Invalid request line format (400)" << RESET << std::endl;
		this->StatusCode = 400;
		return false;
	}

	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		std::cout << SOFT_RED "[ERROR] Method not implemented: " << method << " (501)" << RESET << std::endl;
		this->StatusCode = 501;
		return false;
	}

	if (version != "HTTP/1.1" && version != "HTTP/1.0")
	{
		std::cout << SOFT_RED "[ERROR] HTTP version not supported: " << version << " (505)" << RESET << std::endl;
		this->StatusCode = 505;
		return false;
	}

	if (path.empty() || path[0] != '/')
	{
		std::cout << SOFT_RED "[ERROR] Invalid path: " << path << " (400)" << RESET << std::endl;
		this->StatusCode = 400;
		return false;
	}

	if (path.length() > 8192)
	{
		std::cout << SOFT_RED "[ERROR] URI too long (> 8192 bytes) (414)" << RESET << std::endl;
		this->StatusCode = 414;
		return false;
	}

	this->method = method;
	this->uri = path;
	this->version = version;

	return true;
}

bool HttpRequest::ParseOneHeader(const std::string& line)
{
	size_t ColonPos = line.find(':');

	if (ColonPos == std::string::npos)
	{
		this->StatusCode = 400;
		return false;
	}
	std::string key = line.substr(0, ColonPos);
	for (size_t i = 0; i < key.length(); i++)
		key[i] = std::tolower(key[i]);
	if (key.find(' ') != std::string::npos || key.find('\t') != std::string::npos)
	{
		this->StatusCode = 400;
		return false;
	}

	std::string value = line.substr(ColonPos + 1);

	size_t FirstNonSpace = value.find_first_not_of(" \t");
	if (FirstNonSpace != std::string::npos)
		value = value.substr(FirstNonSpace);
	else
		value = "";
	this->headers[key] = value;
	return true;
}