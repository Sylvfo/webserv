#include "Webserv.hpp"

void HttpRequest::Answerlocal()
{
	if (method == "GET")
	{
		GetRequest();
	}
	else if (method == "POST")
	{
		PostRequest();
	}
	else if (method == "DELETE")
	{
		DeleteRequest();
	}
	
	SetStatusLine();
	SetResponseHeader();
}

void HttpRequest::SetResponseHeader()
{
	// Handle 301 redirect for directories
	if (StatusCode == 301)
	{
		std::string redirectUri = uri;
		if (!redirectUri.empty() && redirectUri[redirectUri.length() - 1] != '/')
			redirectUri += "/";
		
		HttpAnswer += "Location: ";
		HttpAnswer += redirectUri;
		HttpAnswer += "\r\n";
		HttpAnswer += "Content-Length: 0\r\n";
		HttpAnswer += "Connection: close\r\n";
		HttpAnswer += "\r\n";
		return;
	}
	
	// Reserve space for headers + body to avoid reallocation
	size_t estimatedSize = 200 + AnswerBody.size(); // ~200 bytes for headers
	try {
		HttpAnswer.reserve(estimatedSize);
	} catch (const std::bad_alloc& e) {
		// Continue anyway, let it fail naturally if needed
	}
	
	HttpAnswer += "Content-Length: ";
	HttpAnswer += IntToString(ContentLength);
	HttpAnswer += "\r\n";
	HttpAnswer += "Content-Type: ";
	HttpAnswer += ContentType;
	HttpAnswer += "\r\n";
	HttpAnswer += "Connection: close\r\n";
	HttpAnswer += "\r\n";  // Séparation headers/body
	HttpAnswer += AnswerBody;
}

void HttpRequest::SetStatusLine()
{
	HttpAnswer.clear();
	HttpAnswer = "HTTP/1.0 "; //put in constructor
	std::map<int, std::string>::iterator it = this->Server->error_code_message.find(StatusCode);
	if (it == Server->error_code_message.end())
	{
		HttpAnswer += "500 Internal Server Error";
	}
	else
	{
		HttpAnswer += it->second;
	}
	HttpAnswer += "\r\n";
}
