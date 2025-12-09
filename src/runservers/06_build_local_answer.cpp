#include "Webserv.hpp"

void HttpRequest::Answerlocal()
{
	//std::cout << PASTEL_AQUA "Enter local Answer " << RESET << std::endl;

	if (method == "GET")
		GetRequest();
	else if (method == "POST")
		PostRequest();
	else if (method == "DELETE")
		DeleteRequest();
	SetStatusLine();
	SetResponseHeader();
}

//façon plus classe de faire ça??
void HttpRequest::SetResponseHeader()
{
	HttpAnswer += "Content-Length: ";
	HttpAnswer += ContentLenght;
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
	//HttpAnswer += StatusCode;
	std::map<int, std::string>::iterator it = this->Server->error_code_message.find(StatusCode);
	if (it == Server->error_pages.end())
	{
		HttpAnswer +="404 Not Found";
	}
	HttpAnswer += it->second;
	///
	HttpAnswer += "\r\n";

}
