#include "Webserv.hpp"

void WebServ::handleRequest(epoll_event current_event)
{
	ConnectionData* connInfo = static_cast<ConnectionData*>(current_event.data.ptr);
   	if (!connInfo)
    {
        std::cerr << SOFT_RED "[ERROR] NULL connection info" << RESET << std::endl;
        return;
    }
	HttpRequest& request = connInfo->request;
	request.socket_fd = connInfo->client_fd;
	request.Server = connInfo->server;
	if (!request.HeaderComplete)
	{
		if (!request.ReceiveHeader())
		{
			if (request.AnswerType == ERROR)
			{
				request.AnswerError();
				request.sendAnswerToRequest();
				request.RequestComplete = true;
			}
			std::cout << SOFT_RED "[ERROR] Failed to receive header" << RESET << std::endl;
			return;
		}
		if (request.HeaderComplete)
		{
			if (!request.ParseHeader() || !request.ValidateHeader())
			{
				if (request.AnswerType == ERROR)
				{
					request.AnswerError();
					request.sendAnswerToRequest();
					request.RequestComplete = true;
				}
				return;
			}
		}
		else
			return;
	}
	if (request.HeaderComplete && request.ExpectingBody && !request.BodyComplete && request.AnswerType != ERROR)
	{
		if (!request.ReceiveBody())
		{
			std::cout << SOFT_RED "[ERROR] Failed to receive body" << RESET << std::endl;
			if (request.AnswerType == ERROR)
			{
				request.AnswerError();
				request.sendAnswerToRequest();
				request.RequestComplete = true;
			}
			return;
		}
		if (!request.BodyComplete)
			return;
	}
	if (request.AnswerType != ERROR)
		request.CheckRequest();
	if (request.AnswerType == ERROR)
		request.AnswerError();
	else if (request.AnswerType == STATIC)
		request.Answerlocal();
	else if (request.AnswerType == CGI)
		request.AnswerCGI();
	request.sendAnswerToRequest();
	request.RequestComplete = true;
}

void HttpRequest::sendAnswerToRequest()
{
	int bytesSent = 0;
	int totalBytesSent = 0;

	while(totalBytesSent < (int)HttpAnswer.size())
	{
		bytesSent = send(socket_fd, HttpAnswer.c_str(), HttpAnswer.size(), 0);
		if (bytesSent < 0)
		{
			std::cout << SOFT_RED "[ERROR] Could not send response" << RESET << std::endl;
			return;
		}
		totalBytesSent += bytesSent;
	}
}

std::string IntToString(int numb)
{
	std::string value;
	std::stringstream out;

	out << numb;
	value = out.str();
	return (value);
}


