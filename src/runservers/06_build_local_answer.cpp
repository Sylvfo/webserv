#include "Webserv.hpp"

void HttpRequest::Answerlocal()
{
	std::cout << PASTEL_AQUA "[ANSWER_LOCAL] Building local answer for method: " << method << RESET << std::endl;

	if (method == "GET")
	{
		std::cout << PASTEL_AQUA "[ANSWER_LOCAL] Handling GET request" << RESET << std::endl;
		GetRequest();
	}
	else if (method == "POST")
	{
		std::cout << PASTEL_AQUA "[ANSWER_LOCAL] Handling POST request" << RESET << std::endl;
		PostRequest();
	}
	else if (method == "DELETE")
	{
		std::cout << PASTEL_AQUA "[ANSWER_LOCAL] Handling DELETE request" << RESET << std::endl;
		DeleteRequest();
	}
	std::cout << PASTEL_AQUA "[ANSWER_LOCAL] Setting status line..." << RESET << std::endl;
	SetStatusLine();
	std::cout << PASTEL_AQUA "[ANSWER_LOCAL] Setting response headers..." << RESET << std::endl;
	SetResponseHeader();
	std::cout << PASTEL_AQUA "[ANSWER_LOCAL] Local answer complete" << RESET << std::endl;
}

//façon plus classe de faire ça??
void HttpRequest::SetResponseHeader()
{
	std::cout << PASTEL_AQUA "[SET_HEADER] Building response headers" << RESET << std::endl;
	HttpAnswer += "Content-Length: ";
	HttpAnswer += IntToString(ContentLength);
	HttpAnswer += "\r\n";
	HttpAnswer += "Content-Type: ";
	HttpAnswer += ContentType;
	HttpAnswer += "\r\n";
	HttpAnswer += "Connection: close\r\n";
	HttpAnswer += "\r\n";  // Séparation headers/body
	HttpAnswer += AnswerBody;
	std::cout << PASTEL_AQUA "[SET_HEADER] Headers set, total response size: " << HttpAnswer.size() << " bytes" << RESET << std::endl;
}

void HttpRequest::SetStatusLine()
{
	std::cout << PASTEL_AQUA "[SET_STATUS] Setting status line for code " << StatusCode << RESET << std::endl;
	HttpAnswer.clear();
	HttpAnswer = "HTTP/1.0 "; //put in constructor
	//HttpAnswer += StatusCode;
	std::map<int, std::string>::iterator it = this->Server->error_code_message.find(StatusCode);
	if (it == Server->error_pages.end())
	{
		std::cout << SOFT_RED "[SET_STATUS] Status code message not found, using default" << RESET << std::endl;
		HttpAnswer +="404 Not Found";
	}
	else
	{
		std::cout << PASTEL_AQUA "[SET_STATUS] Status message: " << it->second << RESET << std::endl;
	}
	HttpAnswer += it->second;
	///
	HttpAnswer += "\r\n";
	std::cout << PASTEL_AQUA "[SET_STATUS] Status line set: " << HttpAnswer << RESET << std::endl;
}
