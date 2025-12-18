#include "HttpRequest.hpp"

bool HttpRequest::parseHeader()
{
	std::stringstream stream(this->raw_header);
	std::string line;

	if (!std::getline(stream, line))
	{
		this->answer_type = ERROR;
		this->status_code = 400;
		return false;
	}

	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);


	if (!_parseRequestLine(line))
	{
		this->answer_type = ERROR;
		return false;
	}

	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (line.empty())
			break;

		if (!_parseOneHeader(line))
		{
			this->answer_type = ERROR;
			return false;
		}
	}

	if (this->version == "HTTP/1.1" && (this->headers.find("host") == this->headers.end()))
	{
		this->answer_type = ERROR;
		this->status_code = 400;
		return false;
	}
	return true;
}

bool HttpRequest::_parseRequestLine(const std::string& line)
{
	std::cout << line << std::endl;
	std::stringstream stream(line);
	std::string method, path, version, junk;

	stream >> method >> path >> version;
	if (method.empty() || path.empty() || version.empty() || (stream >> junk))
	{
		this->status_code = 400;
		return false;
	}

	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		this->status_code = 501;
		return false;
	}

	if (version != "HTTP/1.1" && version != "HTTP/1.0")
	{
		this->status_code = 505;
		return false;
	}

	if (path.empty() || path[0] != '/')
	{
		this->status_code = 400;
		return false;
	}

	if (path.length() > 8192)
	{
		this->status_code = 414;
		return false;
	}

	this->method = method;
	this->uri = path;
	this->version = version;

	return true;
}

bool HttpRequest::_parseOneHeader(const std::string& line)
{
	size_t ColonPos = line.find(':');

	if (ColonPos == std::string::npos)
	{
		this->status_code = 400;
		return false;
	}
	std::string key = line.substr(0, ColonPos);
	for (size_t i = 0; i < key.length(); i++)
		key[i] = std::tolower(static_cast<unsigned char>(key[i]));
	if (key.find(' ') != std::string::npos || key.find('\t') != std::string::npos)
	{
		this->status_code = 400;
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
