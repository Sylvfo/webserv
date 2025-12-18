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
	if (!request.header_complete)
	{
		if (!request.receiveHeader())
		{
			if (request.answer_type == ERROR)
			{
				request.answerError();
				request.sendAnswerToRequest();
				request.request_complete = true;
			}
			std::cout << SOFT_RED "[ERROR] Failed to receive header" << RESET << std::endl;
			return;
		}
		if (request.header_complete)
		{
			if (!request.parseHeader() || !request.validateHeader())
			{
				if (request.answer_type == ERROR)
				{
					request.answerError();
					request.sendAnswerToRequest();
					request.request_complete = true;
				}
				return;
			}
		}
		else
			return;
	}
	if (request.header_complete && request.expecting_body && !request.body_complete && request.answer_type != ERROR)
	{
		if (!request.receiveBody())
		{
			std::cout << SOFT_RED "[ERROR] Failed to receive body" << RESET << std::endl;
			if (request.answer_type == ERROR)
			{
				request.answerError();
				request.sendAnswerToRequest();
				request.request_complete = true;
			}
			return;
		}
		if (!request.body_complete)
			return;
	}
	if (request.answer_type != ERROR)
		request.checkRequest();
	if (request.answer_type == ERROR)
		request.answerError();
	else if (request.answer_type == STATIC)
		request.answerLocal();
	else if (request.answer_type == CGI)
		request.answerCGI();
	request.sendAnswerToRequest();
	request.request_complete = true;
}

void HttpRequest::sendAnswerToRequest()
{
	ssize_t totalBytesSent = 0;

	std::cout << "[DEBUG] Sending http_answer.size() = " << http_answer.size() << " bytes" << std::endl;

	while(totalBytesSent < (int)http_answer.size())
	{
		ssize_t bytesSent = send(socket_fd, http_answer.data() + totalBytesSent, http_answer.size() - totalBytesSent, MSG_NOSIGNAL);
		if (bytesSent > 0)
			totalBytesSent += bytesSent;
		else if (bytesSent == 0)
			return;
		else if (bytesSent < 0)
			continue;
	}
	std::cout << "[INFO] Successfully sent " << totalBytesSent << " bytes" << std::endl;
}

std::string intToString(int numb)
{
	std::string value;
	std::stringstream out;

	out << numb;
	value = out.str();
	return (value);
}


