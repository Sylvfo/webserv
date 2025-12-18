#include "HttpRequest.hpp"

bool HttpRequest::validateHeader() // a helper function for parseHeader if parseHeader would grow to big
{
	this->is_chunked = false;
	this->expecting_body = false;
	this->content_length = 0;

	if (this->headers.count("transfer-encoding"))
	{
		if (this->headers["transfer-encoding"].find("chunked") != std::string::npos)
		{
			this->is_chunked = true;
			this->status_code = 501;
			this->answer_type = ERROR;
			return false;
		}
	}

	if (this->headers.count("content-length"))
	{
		//const char* value_str = this->headers["content-length"].c_str();
		//char* endptr;
		//unsigned long parsed = std::strtoul(value_str, &endptr, 10);
		std::stringstream stream(this->headers["content-length"]);
		unsigned long parsed;

		char leftover;
		if ((!(stream >> parsed)) || stream >> leftover)
		//if (*endptr != '\0' || endptr == value_str)
		{
			this->status_code = 400;
			this->answer_type = ERROR;
			return false;
		}
		this->content_length = static_cast<size_t>(parsed);

		if (this->content_length > this->Server->client_max_body_size)
		{
			this->status_code = 413;
			this->answer_type = ERROR;
			return false;
		}
		this->expecting_body = true;
		return true;
	}
	if (this->method == "POST")
	{
		this->status_code = 411;
		this->answer_type = ERROR;
		return false;
	}
	return true;
}



void HttpRequest::checkRequest()
{
	std::string uriWithoutQuery = this->uri;
	size_t queryPos = uriWithoutQuery.find('?');
	if (queryPos != std::string::npos)
	{
		uriWithoutQuery = uriWithoutQuery.substr(0, queryPos);
	}

	size_t LongestMatch = 0;
	int MatchedIndex = -1;
	std::string uri = _urlDecode(uriWithoutQuery);

	std::string root = Server->root;

	for (size_t i = 0; i < Server->locations.size(); i++)
	{
		const std::string& LocationPath = Server->locations[i].path;
		if (uri.find(LocationPath) == 0)
		{
			size_t matchLen = LocationPath.length();
			if (uri.length() == matchLen ||
			    uri[matchLen] == '/' ||
			    LocationPath[LocationPath.length() - 1] == '/')
			{
				if (LocationPath.length() > LongestMatch)
				{
					LongestMatch = LocationPath.length();
					MatchedIndex = i;
				}
			}
		}
	}

	if (MatchedIndex != -1)
	{
		bool MethodAllowed = false;
		std::vector<std::string>& allowed = Server->locations[MatchedIndex].methods;
		for (size_t j = 0; j < allowed.size(); j++)
		{
			if (allowed[j] == this->method)
			{
				MethodAllowed = true;
				break;
			}
		}
		if (!MethodAllowed)
		{
			this->status_code = 405;
			this->answer_type = ERROR;
			return;
		}
		if (!Server->locations[MatchedIndex].root.empty())
		{
			root = Server->locations[MatchedIndex].root;
		}
	}
	std::string RelativePath;
	if (MatchedIndex != -1)
		RelativePath = uri.substr(Server->locations[MatchedIndex].path.length());
	else
		RelativePath = uri;

	if (!root.empty() && root[root.length() - 1] == '/' && !RelativePath.empty() && RelativePath[0] == '/')
	{
		RelativePath = RelativePath.substr(1);
	}

	if (root[root.length() - 1] == '/')
		this ->path= root + RelativePath;
	else
		this ->path= root + "/" + RelativePath;

    if (access(this->path.c_str(), F_OK) != 0)
	{
		this->status_code = 404;
		this->answer_type = ERROR;
		return;
	}
	
	if (access(this->path.c_str(), R_OK) != 0)
	{
		this->status_code = 403;
		this->answer_type = ERROR;
		return;
	}
	
	struct stat FileInfo;
	if (stat(this->path.c_str(), &FileInfo) != 0)
	{
		this->status_code = 500;
		this->answer_type = ERROR;
		return;
	}

	if (S_ISDIR(FileInfo.st_mode))
	{
		this->is_directory = true;
		if (this->uri[uri.length() - 1] != '/')
		{
			this->status_code = 301;
			this->redirection_url = this->uri + "/";
			this->answer_type = ERROR;
			return;
		}
	}
	else
	{
		this->is_directory = false;
		if (this->uri.length() > 0 && this->uri[this->uri.length() -1] == '/')
		{
			this->uri.erase(this->uri.length() - 1);
		}
	}

	this->status_code = 200;
	if (MatchedIndex != -1)
	{
		CGIHandler cgiHandler;
		if (cgiHandler.isCGI(uri, Server->locations[MatchedIndex]))
		{
			this->answer_type = CGI;
			return;
		}
	}

	this->answer_type = STATIC;
}

std::string HttpRequest::_urlDecode(const std::string& str)
{
	std::string result;

	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '+')
		{
			result += ' ';
		}
		else if (str[i] == '%' && i + 2 < str.length())
		{
			std::string hexStr = str.substr(i + 1, 2);
			//char* endPtr;
			std::stringstream sHexValue;
			sHexValue << std::hex << hexStr;
			long hexValue;
			sHexValue >> hexValue;
			if (!sHexValue.eof())
			{
				result += static_cast<char>(hexValue);
				i += 2;
			}
			// long hexValue = strtol(hexStr.c_str(), &endPtr, 16);
			// if (*endPtr == '\0')
			// {
			// 	result += static_cast<char>(hexValue);
			// 	i += 2;
			// }
			else
			{
				result += str[i];
			}
		}
		else
		{
			result += str[i];
		}
	}
	return result;
}
