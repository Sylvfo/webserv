
#include "HttpRequest.hpp"
#include "CGI.hpp"

// check what is allwoed and replace
#include <sys/socket.h>
#include <sstream>
#include <cstdlib>
#include <iostream>
#include <cerrno>
#include <sys/stat.h> // for stat

// Constructor
HttpRequest::HttpRequest()
	// --- Server & Connection ---
	: Server(NULL)
	, socket_fd(-1)

	// --- Request State ---
	, HeaderComplete(false)

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

// Destructor
HttpRequest::~HttpRequest()
{
}

// HTTP Header Parsing Functions
// These replace 04_recive_request_.cpp and RawHeader.cpp

bool HttpRequest::ReceiveHeader()
{
	std::vector<char> temp_buffer(RECEIVE_CHUNK_SIZE);
	ssize_t bytes_received = recv(socket_fd, &temp_buffer[0], temp_buffer.size(), 0);

	if (bytes_received > 0)
	{
		this->PartialRequest.append(&temp_buffer[0], bytes_received);

		// RFC 7230: Reject headers exceeding reasonable size to prevent DoS
		if (this->PartialRequest.size() > MAX_HEADER_SIZE)
		{
			this->AnswerType = ERROR;
			this->StatusCode = 431;  // Request Header Fields Too Large
			return false;
		}

		size_t seperator_pos = this->PartialRequest.find("\r\n\r\n");
		if (seperator_pos != std::string::npos)
		{
			this->RawHeader = this->PartialRequest.substr(0, seperator_pos);
			this->PartialBody = this->PartialRequest.substr(seperator_pos + 4);
			this->HeaderComplete = true;
			this->PartialRequest.clear();
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
		this->AnswerType = ERROR;
		this->StatusCode = 400; // empty request
		return false;
	}

	// remove trailing \r

	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);

	if (!ParseRequestLine(line))
	{
		this->AnswerType = ERROR;
		// status code is set in helper
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
			// ststud code is set in helper function
			this->AnswerType = ERROR;
			return false;
		}
	}

	if (this->version == "HTTP/1.1" && (this->headers.find("host") == this->headers.end()))
	{
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
		this->StatusCode = 400;// bad request
		return false;
	}

	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		this->StatusCode = 501; // not implemented
		return false;
	}

	if (version != "HTTP/1.1" && version != "HTTP/1.0")
	{
		this->StatusCode = 505; //http version not supported
		return false;
	}

	if (path.empty() || path[0] != '/')
	{
		this->StatusCode = 400; // bad request
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
	this->IsChunked = false;
	this->ExpectingBody = false;
	this->ContentLength = 0;

	// RFC 7230: Transfer-Encoding header indicates chunked encoding
	// This is HTTP/1.1 specific but we support it for compatibility
	if (this->headers.count("transfer-encoding"))
	{
		if (this->headers["transfer-encoding"].find("chunked") != std::string::npos)
		{
			this->IsChunked = true;
			this->ExpectingBody = true;
			return true;
		}
	}

	// RFC 7230: Content-Length header indicates body size in bytes
	if (this->headers.count("content-length"))
	{
		const char* value_str = this->headers["content-length"].c_str();
		char* endptr;

		// strtoul = string to unsigned long
		// Converts "12345" to number 12345 (base 10)
		// endptr will point to first non-digit character
		unsigned long parsed = std::strtoul(value_str, &endptr, 10);

		// Validate: entire string must be converted (no invalid characters)
		// RFC 7230: Content-Length must be decimal number only
		if (*endptr != '\0' || endptr == value_str)
		{
			this->StatusCode = 400; // Bad Request - invalid Content-Length
			this->AnswerType = ERROR;
			return false;
		}

		this->ContentLength = static_cast<size_t>(parsed);
		if (this->ContentLength > this->Server->client_max_body_size)
		{
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
		this->StatusCode = 411; //Length Required
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

	std::vector<char> buffer(RECEIVE_CHUNK_SIZE);

	ssize_t bytes = recv(socket_fd, &buffer[0], buffer.size(), 0);

	if (bytes > 0)
	{
		this->RawBody.append(&buffer[0], bytes);
		if (this->RawBody.size() > this->Server->client_max_body_size)
		{
			this->StatusCode = 413;
			this->AnswerType = ERROR;
			return false;
		}
	}
	
	else if (bytes == 0)
		return false; // Connection closed or error

	if (!this->IsChunked)
	{
		if (this->RawBody.size() >= this->ContentLength)
		{
			if (this->RawBody.size() > this->ContentLength)
				this->RawBody = this->RawBody.substr(0, this->ContentLength);
			this->BodyComplete = true;
			return true; // Body should be complete now
		}
	}
	return true; // Still receiving, not an error
}

void HttpRequest::CheckRequest()
{
	// find longest match
	size_t LongestMatch = 0;
	int MatchedIndex = -1;
	std::string uri = urlDecode(this->uri);

	//defaults to server root if nothing else matches
	std::string root = Server->root;

	for (size_t i = 0; i < Server->locations.size(); i++)
	{
		const std::string& LocationPath = Server->locations[i].path;
		if (uri.find(LocationPath) == 0)
		{
			if (LocationPath.length() > LongestMatch)
			{
				LongestMatch = LocationPath.length();
				MatchedIndex = i;
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
			this->StatusCode = 405; // method nod allowed
			this->AnswerType = ERROR;
			return;
		}
		if (!Server->locations[MatchedIndex].root.empty())
			root = Server->locations[MatchedIndex].root;
	}

	// construct Path
	std::string RelativePath;
	if (MatchedIndex != -1)
		RelativePath = uri.substr(Server->locations[MatchedIndex].path.length());
	else
		RelativePath = uri;

	// alias logic to implement!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	if (root[root.length() - 1] == '/')
		this->Path = root + RelativePath;
	else
		this->Path = root + "/" + RelativePath;

	// Check existence
	struct stat FileInfo;
	if (stat(this->Path.c_str(), &FileInfo) != 0)
	{
		this->StatusCode = 404;
		this->AnswerType = ERROR;
		return;
	}

	this->StatusCode = 200; //Success

	// Check for CGI
	if (MatchedIndex != -1)
	{
		CGIHandler cgiHandler;
		if (cgiHandler.isCGI(uri, Server->locations[MatchedIndex]))
		{
			std::cout << "[CGI] Detected CGI request: " << uri << std::endl;
			this->AnswerType = CGI;
			return;
		}
	}
	
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
