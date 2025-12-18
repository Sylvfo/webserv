#include "HttpRequest.hpp"

bool HttpRequest::validateHeader() // a helper function for parseHeader if parseHeader would grow to big
{
	std::cout << LIGHT_CYAN "[VALIDATE_HEADER] Validating header..." << RESET << std::endl;
	this->is_chunked = false;
	this->expecting_body = false;
	this->content_length = 0;

	if (this->headers.count("transfer-encoding"))
	{
		if (this->headers["transfer-encoding"].find("chunked") != std::string::npos)
		{
			std::cout << SOFT_RED "[ERROR] Chunked transfer encoding not supported (501)" << RESET << std::endl;
			this->is_chunked = true;
			this->status_code = 501;
			this->answer_type = ERROR;
			return false;
		}
	}

	if (this->headers.count("content-length"))
	{
		const char* value_str = this->headers["content-length"].c_str();
		char* endptr;
		unsigned long parsed = std::strtoul(value_str, &endptr, 10);

		if (*endptr != '\0' || endptr == value_str)
		{
			std::cout << SOFT_RED "[ERROR] Invalid Content-Length format (400)" << RESET << std::endl;
			this->status_code = 400;
			this->answer_type = ERROR;
			return false;
		}
		this->content_length = static_cast<size_t>(parsed);

		if (this->content_length > this->Server->client_max_body_size)
		{
			std::cout << SOFT_RED "[ERROR] Content-Length exceeds max (" << this->Server->client_max_body_size << " bytes) (413)" << RESET << std::endl;
			this->status_code = 413;
			this->answer_type = ERROR;
			return false;
		}
		this->expecting_body = true;
		return true;
	}
	if (this->method == "POST")
	{
		std::cout << SOFT_RED "[ERROR] POST requires Content-Length or Transfer-Encoding (411)" << RESET << std::endl;
		this->status_code = 411;
		this->answer_type = ERROR;
		return false;
	}
	return true;
}



void HttpRequest::checkRequest()
{
	std::cout << LIGHT_CYAN "[CHECK_REQUEST] Checking request for URI: " << this->uri << RESET << std::endl;

	std::string uriWithoutQuery = this->uri;
	size_t queryPos = uriWithoutQuery.find('?');
	if (queryPos != std::string::npos)
	{
		uriWithoutQuery = uriWithoutQuery.substr(0, queryPos);
		std::cout << LIGHT_CYAN "[CHECK_REQUEST] Stripped query string, URI: " << uriWithoutQuery << RESET << std::endl;
	}

	size_t LongestMatch = 0;
	int MatchedIndex = -1;
	std::string uri = _urlDecode(uriWithoutQuery);
	std::cout << LIGHT_CYAN "[CHECK_REQUEST] URL decoded URI: " << uri << RESET << std::endl;

	std::string root = Server->root;

	std::cout << LIGHT_CYAN "[CHECK_REQUEST] Searching for matching location..." << RESET << std::endl;
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
					std::cout << LIGHT_CYAN "[CHECK_REQUEST] Found match: " << LocationPath << " (length: " << LocationPath.length() << ")" << RESET << std::endl;
				}
			}
		}
	}

	if (MatchedIndex != -1)
	{
		std::cout << LIGHT_CYAN "[CHECK_REQUEST] Best match: " << Server->locations[MatchedIndex].path << RESET << std::endl;
		std::cout << LIGHT_CYAN "[CHECK_REQUEST] Checking if method " << this->method << " is allowed..." << RESET << std::endl;
		bool MethodAllowed = false;
		std::vector<std::string>& allowed = Server->locations[MatchedIndex].methods;
		for (size_t j = 0; j < allowed.size(); j++)
		{
			if (allowed[j] == this->method)
			{
				MethodAllowed = true;
				std::cout << LIGHT_CYAN "[CHECK_REQUEST] Method allowed" << RESET << std::endl;
				break;
			}
		}
		if (!MethodAllowed)
		{
			std::cout << SOFT_RED "[CHECK_REQUEST] Method not allowed (405)" << RESET << std::endl;
			this->status_code = 405;
			this->answer_type = ERROR;
			return;
		}
		if (!Server->locations[MatchedIndex].root.empty())
		{
			root = Server->locations[MatchedIndex].root;
			std::cout << LIGHT_CYAN "[CHECK_REQUEST] Using location root: " << root << RESET << std::endl;
		}
	}
	else
	{
		std::cout << LIGHT_CYAN "[CHECK_REQUEST] No location match, using server root" << RESET << std::endl;
	}
	std::string RelativePath;
	if (MatchedIndex != -1)
		RelativePath = uri.substr(Server->locations[MatchedIndex].path.length());
	else
		RelativePath = uri;

	if (!root.empty() && root[root.length() - 1] == '/' && !RelativePath.empty() && RelativePath[0] == '/')
	{
		RelativePath = RelativePath.substr(1);
		std::cout << LIGHT_CYAN "[CHECK_REQUEST] Removed leading slash from relative path" << RESET << std::endl;
	}

	if (root[root.length() - 1] == '/')
		this ->path= root + RelativePath;
	else
		this ->path= root + "/" + RelativePath;

	std::cout << LIGHT_CYAN "[CHECK_REQUEST] Constructed path: " << this->path<< RESET << std::endl;
	std::cout << LIGHT_CYAN "[CHECK_REQUEST] Checking if path exists..." << RESET << std::endl;

    if (access(this->path.c_str(), F_OK) != 0)
	{
		std::cout << SOFT_RED "[CHECK_REQUEST] path not found (404)" << RESET << std::endl;
		this->status_code = 404;
		this->answer_type = ERROR;
		return;
	}
	
	if (access(this->path.c_str(), R_OK) != 0)
	{
		std::cout << SOFT_RED "[CHECK_REQUEST] Permission denied (403)" << RESET << std::endl;
		this->status_code = 403;
		this->answer_type = ERROR;
		return;
	}
	
	std::cout << LIGHT_CYAN "[CHECK_REQUEST] path exists and is readable" << RESET << std::endl;
	
	struct stat FileInfo;
	if (stat(this->path.c_str(), &FileInfo) != 0)
	{
		std::cout << SOFT_RED "[CHECK_REQUEST] Failed to stat path" << RESET << std::endl;
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
			std::cout << BRIGHT_MAGENTA "[CHECK_REQUEST] Detected CGI request: " << uri << RESET << std::endl;
			this->answer_type = CGI;
			return;
		}
	}

	std::cout << LIGHT_CYAN "[CHECK_REQUEST] Setting answer type to STATIC" << RESET << std::endl;
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
			char* endPtr;
			long hexValue = strtol(hexStr.c_str(), &endPtr, 16);
			if (*endPtr == '\0')
			{
				result += static_cast<char>(hexValue);
				i += 2;
			}
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