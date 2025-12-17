
#include "HttpRequest.hpp"
#include "CGI.hpp"
#include "colors.hpp"
#include <sys/socket.h>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>

HttpRequest::HttpRequest()
	// --- Server & Connection ---
	: Server(NULL)
	, socket_fd(-1)

	// --- Request State ---
	, HeaderComplete(false)
	, RequestComplete(false)

	// --- Body Handling ---
	, ExpectingBody(false)
	, IsChunked(false)
	, BodyComplete(false)
	, ContentLength(0)

	// --- Response ---
	, AnswerType(STATIC)
	, StatusCode(200)
	, fd_Ressource(-1)
{
}

HttpRequest::HttpRequest(const HttpRequest& other)
	: Server(other.Server)
	, socket_fd(other.socket_fd)
	, method(other.method)
	, uri(other.uri)
	, version(other.version)
	, Path(other.Path)
	, headers(other.headers)
	, HeaderComplete(other.HeaderComplete)
	, PartialRequest(other.PartialRequest)
	, RawHeader(other.RawHeader)
	, RequestComplete(other.RequestComplete)
	, ExpectingBody(other.ExpectingBody)
	, IsChunked(other.IsChunked)
	, BodyComplete(other.BodyComplete)
	, ContentLength(other.ContentLength)
	, RawBody(other.RawBody)
	, PartialBody(other.PartialBody)
	, AnswerType(other.AnswerType)
	, StatusCode(other.StatusCode)
	, HttpAnswer(other.HttpAnswer)
	, ContentType(other.ContentType)
	, AnswerBody(other.AnswerBody)
	, fd_Ressource(other.fd_Ressource)
{
}

HttpRequest& HttpRequest::operator=(const HttpRequest& other)
{
	if (this != &other)
	{
		Server = other.Server;
		socket_fd = other.socket_fd;
		method = other.method;
		uri = other.uri;
		version = other.version;
		Path = other.Path;
		headers = other.headers;
		HeaderComplete = other.HeaderComplete;
		PartialRequest = other.PartialRequest;
		RawHeader = other.RawHeader;
		RequestComplete = other.RequestComplete;
		ExpectingBody = other.ExpectingBody;
		IsChunked = other.IsChunked;
		BodyComplete = other.BodyComplete;
		ContentLength = other.ContentLength;
		RawBody = other.RawBody;
		PartialBody = other.PartialBody;
		AnswerType = other.AnswerType;
		StatusCode = other.StatusCode;
		HttpAnswer = other.HttpAnswer;
		ContentType = other.ContentType;
		AnswerBody = other.AnswerBody;
		fd_Ressource = other.fd_Ressource;
	}
	return *this;
}

HttpRequest::~HttpRequest()
{
}

bool HttpRequest::ReceiveHeader()
{
	std::vector<char> temp_buffer(MAX_HEADER_SIZE);
	ssize_t bytes_received = recv(socket_fd, &temp_buffer[0], temp_buffer.size(), 0);

	if (bytes_received > 0)
	{
		this->PartialRequest.append(&temp_buffer[0], bytes_received);

		// RFC 7230: Reject headers exceeding reasonable size to prevent DoS
		if (this->PartialRequest.size() > MAX_HEADER_SIZE)
		{
			std::cout << SOFT_RED "[RECEIVE_HEADER] 431" << RESET << std::endl;
			this->AnswerType = ERROR;
			this->StatusCode = 431;  // Request Header Fields Too Large
			return false;
		}

		size_t seperator_pos = this->PartialRequest.find("\r\n\r\n");
		if (seperator_pos != std::string::npos)
		{
			std::cout << SOFT_RED "[RECEIVE_HEADER] seperator found" << RESET << std::endl;
			this->RawHeader = this->PartialRequest.substr(0, seperator_pos);
			this->PartialBody = this->PartialRequest.substr(seperator_pos + 4);
			this->HeaderComplete = true;
			this->PartialRequest.clear();
		}
		else if (seperator_pos == std::string::npos)// this means the header is too long
		{
			std::cout << SOFT_RED "[RECEIVE_HEADER] uri too long" << RESET << std::endl;
			this->AnswerType = ERROR;
			this->StatusCode = 414; //uri too long
			return false;
		}
		return true;
	}
	if (bytes_received == 0)
		return false;
	return true;
}

bool HttpRequest::ParseHeader()
{
	std::stringstream stream(this->RawHeader);
	std::string line;

	// line parsing

	if (!std::getline(stream, line))
	{
		std::cout << SOFT_RED "[PARSE_HEADER] Empty request (400)" << RESET << std::endl;
		this->AnswerType = ERROR;
		this->StatusCode = 400; // empty request
		return false;
	}

	// remove trailing \r

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

		// parse into map
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
		value = ""; // just empty value because it's not forbidden
	this->headers[key] = value;
	return true;
}

bool HttpRequest::ValidateHeader() // a helper function for ParseHeader if ParseHeader would grow to big
{
	std::cout << LIGHT_CYAN "[VALIDATE_HEADER] Validating header..." << RESET << std::endl;
	this->IsChunked = false;
	this->ExpectingBody = false;
	this->ContentLength = 0;

	// RFC 7230: Transfer-Encoding header indicates chunked encoding
	// This is HTTP/1.1 specific but we support it for compatibility
	if (this->headers.count("transfer-encoding"))
	{
		if (this->headers["transfer-encoding"].find("chunked") != std::string::npos)
		{
			std::cout << SOFT_RED "[ERROR] Chunked transfer encoding not supported (501)" << RESET << std::endl;
			this->IsChunked = true;
			this->StatusCode = 501;
			this->AnswerType = ERROR;
			return false;
		}
	}

	// RFC 7230: Content-Length header indicates body size in bytes
	if (this->headers.count("content-length"))
	{
		const char* value_str = this->headers["content-length"].c_str();
		char* endptr;

		unsigned long parsed = std::strtoul(value_str, &endptr, 10);

		if (*endptr != '\0' || endptr == value_str)
		{
			std::cout << SOFT_RED "[ERROR] Invalid Content-Length format (400)" << RESET << std::endl;
			this->StatusCode = 400;
			this->AnswerType = ERROR;
			return false;
		}

		this->ContentLength = static_cast<size_t>(parsed);

		if (this->ContentLength > this->Server->client_max_body_size)
		{
			std::cout << SOFT_RED "[ERROR] Content-Length exceeds max (" << this->Server->client_max_body_size << " bytes) (413)" << RESET << std::endl;
			this->StatusCode = 413;
			this->AnswerType = ERROR;
			return false;
		}
		this->ExpectingBody = true;
		return true;
	}

	// RFC 7231: POST requests require Content-Length or Transfer-Encoding
	if (this->method == "POST")
	{
		std::cout << SOFT_RED "[ERROR] POST requires Content-Length or Transfer-Encoding (411)" << RESET << std::endl;
		this->StatusCode = 411;
		this->AnswerType = ERROR;
		return false;
	}

	return true;
}

bool HttpRequest::ReceiveBody()
{
	this->BodyComplete = false;
	// probably stop using partial body just directly raw body
	if (!this->PartialBody.empty())
	{
		this->RawBody.append(this->PartialBody);
		this->PartialBody.clear();
	}

	// For edge-triggered epoll, we must read ALL available data in a loop
	// until we get EAGAIN/EWOULDBLOCK
	std::vector<char> buffer(RECEIVE_CHUNK_SIZE);

	while (true)
	{
		ssize_t bytes = recv(socket_fd, &buffer[0], buffer.size(), 0);

		if (bytes > 0)
		{
			this->RawBody.append(&buffer[0], bytes);

			if (this->RawBody.size() > this->Server->client_max_body_size)
			{
				std::cout << SOFT_RED "[ERROR] Body exceeds max size (413)" << RESET << std::endl;
				this->StatusCode = 413;
				this->AnswerType = ERROR;
				return false;
			}

			// Check if body is complete
			if (!this->IsChunked && this->RawBody.size() >= this->ContentLength)
			{
				if (this->RawBody.size() > this->ContentLength)
					this->RawBody = this->RawBody.substr(0, this->ContentLength);
				this->BodyComplete = true;
				std::cout << LIGHT_CYAN "[BODY] Complete (" << this->RawBody.size() << " bytes)" << RESET << std::endl;
				return true;
			}

			// Continue reading more data (edge-triggered mode)
			continue;
		}
		else if (bytes == 0)
		{
			std::cout << SOFT_RED "[ERROR] Connection closed during body" << RESET << std::endl;
			return false;
		}
		else // bytes < 0
		{
			// Check if it's just no more data available (EAGAIN/EWOULDBLOCK)
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// Check if we already have the complete body
				if (!this->IsChunked && this->RawBody.size() >= this->ContentLength)
				{
					if (this->RawBody.size() > this->ContentLength)
						this->RawBody = this->RawBody.substr(0, this->ContentLength);
					this->BodyComplete = true;
					std::cout << LIGHT_CYAN "[BODY] Complete (" << this->RawBody.size() << " bytes)" << RESET << std::endl;
					return true;
				}

				// Body not complete yet, will continue on next epoll event
				return true;
			}
			else
			{
				std::cout << SOFT_RED "[ERROR] recv() error: " << strerror(errno) << RESET << std::endl;
				return false;
			}
		}
	}

	return true;
}

void HttpRequest::CheckRequest()
{
	std::cout << LIGHT_CYAN "[CHECK_REQUEST] Checking request for URI: " << this->uri << RESET << std::endl;

	// Strip query string from URI for path construction
	std::string uriWithoutQuery = this->uri;
	size_t queryPos = uriWithoutQuery.find('?');
	if (queryPos != std::string::npos)
	{
		uriWithoutQuery = uriWithoutQuery.substr(0, queryPos);
		std::cout << LIGHT_CYAN "[CHECK_REQUEST] Stripped query string, URI: " << uriWithoutQuery << RESET << std::endl;
	}

	// find longest match
	size_t LongestMatch = 0;
	int MatchedIndex = -1;
	std::string uri = urlDecode(uriWithoutQuery);
	std::cout << LIGHT_CYAN "[CHECK_REQUEST] URL decoded URI: " << uri << RESET << std::endl;

	//defaults to server root if nothing else matches
	std::string root = Server->root;

	std::cout << LIGHT_CYAN "[CHECK_REQUEST] Searching for matching location..." << RESET << std::endl;
	for (size_t i = 0; i < Server->locations.size(); i++)
	{
		const std::string& LocationPath = Server->locations[i].path;
		if (uri.find(LocationPath) == 0)
		{
			if (LocationPath.length() > LongestMatch)
			{
				LongestMatch = LocationPath.length();
				MatchedIndex = i;
				std::cout << LIGHT_CYAN "[CHECK_REQUEST] Found match: " << LocationPath << " (length: " << LocationPath.length() << ")" << RESET << std::endl;
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
			this->StatusCode = 405; // method nod allowed
			this->AnswerType = ERROR;
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

	// construct Path
	std::string RelativePath;
	if (MatchedIndex != -1)
		RelativePath = uri.substr(Server->locations[MatchedIndex].path.length());
	else
		RelativePath = uri;

	// Remove leading slash from RelativePath if root ends with slash to avoid double slash
	if (!root.empty() && root[root.length() - 1] == '/' && !RelativePath.empty() && RelativePath[0] == '/')
	{
		RelativePath = RelativePath.substr(1);
		std::cout << LIGHT_CYAN "[CHECK_REQUEST] Removed leading slash from relative path" << RESET << std::endl;
	}

	// alias logic to implement!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	if (root[root.length() - 1] == '/')
		this->Path = root + RelativePath;
	else
		this->Path = root + "/" + RelativePath;

	std::cout << LIGHT_CYAN "[CHECK_REQUEST] Constructed path: " << this->Path << RESET << std::endl;

	// Check existence
	std::cout << LIGHT_CYAN "[CHECK_REQUEST] Checking if path exists..." << RESET << std::endl;
	struct stat FileInfo;
	if (stat(this->Path.c_str(), &FileInfo) != 0)
	{
		std::cout << SOFT_RED "[CHECK_REQUEST] Path not found (404)" << RESET << std::endl;
		this->StatusCode = 404;
		this->AnswerType = ERROR;
		return;
	}
	std::cout << LIGHT_CYAN "[CHECK_REQUEST] Path exists" << RESET << std::endl;

	if (S_ISDIR(FileInfo.st_mode))
	{
		this->IsDirectory = true;
		if (this->uri[uri.length() - 1] != '/')
		{
			this->StatusCode = 301;
			this->RedirectionUrl = this->uri + "/";
			this->AnswerType = ERROR;
			return; // Important: Stop further processing!
		}
	}
	else
	{
		this->IsDirectory = false;
		if (this->uri.length() > 0 && this->uri[this->uri.length() -1] == '/')
		{
			this->uri.erase(this->uri.length() - 1);
		}
	}
	//maybe add access checker

	this->StatusCode = 200; //Success

	// Check for CGI
	if (MatchedIndex != -1)
	{
		CGIHandler cgiHandler;
		if (cgiHandler.isCGI(uri, Server->locations[MatchedIndex]))
		{
			std::cout << BRIGHT_MAGENTA "[CHECK_REQUEST] Detected CGI request: " << uri << RESET << std::endl;
			this->AnswerType = CGI;
			return;
		}
	}

	std::cout << LIGHT_CYAN "[CHECK_REQUEST] Setting answer type to STATIC" << RESET << std::endl;
	this->AnswerType = STATIC;
}

std::string HttpRequest::urlDecode(const std::string& str)
{
	std::string result;

	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '+')
		{
			result += ' ';  // + means space in forms
		}
		else if (str[i] == '%' && i + 2 < str.length())
		{
			// Convert %XX to character
			std::string hexStr = str.substr(i + 1, 2);
			char* endPtr;
			long hexValue = strtol(hexStr.c_str(), &endPtr, 16);
			if (*endPtr == '\0')  // Valid hex
			{
				result += static_cast<char>(hexValue);
				i += 2;  // Skip XX
			}
			else
			{
				result += str[i];  // Keep original if invalid
			}
		}
		else
		{
			result += str[i];
		}
	}

	return result;
}
// to add:
// getter functions
