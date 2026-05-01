#include "Webserv.hpp"

void HttpRequest::answerLocal()
{
	if (method == "GET")
	{
		getRequest();
	}
	else if (method == "POST")
	{
		postRequest();
	}
	else if (method == "DELETE")
	{
		deleteRequest();
	}
	_setStatusLine();
	_setResponseHeader();
}

void HttpRequest::_setResponseHeader()
{
	if (status_code == 301)
	{
		std::string redirectUri = uri;
		if (!redirectUri.empty() && redirectUri[redirectUri.length() - 1] != '/')
			redirectUri += "/";
		
		http_answer += "Location: ";
		http_answer += redirectUri;
		http_answer += "\r\n";
		http_answer += "Content-Length: 0\r\n";
		http_answer += "Connection: close\r\n";
		http_answer += "\r\n";
		return;
	}
	size_t estimatedSize = 200 + answer_body.size();
	try {
		http_answer.reserve(estimatedSize);
	} catch (const std::bad_alloc& e) {
	}
	
	http_answer += "Content-Length: ";
	http_answer += intToString(content_length);
	http_answer += "\r\n";
	http_answer += "Content-Type: ";
	http_answer += content_type;
	http_answer += "\r\n";
	http_answer += "Connection: close\r\n";
	http_answer += "\r\n";
	http_answer += answer_body;
}

void HttpRequest::_setStatusLine()
{
	http_answer.clear();
	http_answer = "HTTP/1.0 ";
	std::map<int, std::string>::iterator it = this->Server->error_code_message.find(status_code);
	if (it == Server->error_code_message.end())
	{
		http_answer += "501 Not Implemented";
	}
	else
	{
		http_answer += it->second;
	}
	http_answer += "\r\n";
}