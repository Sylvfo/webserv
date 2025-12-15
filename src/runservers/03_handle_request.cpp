#include "Webserv.hpp"

//check request avec port et nom de domaine ici ou aprse?

/*TEMP FUNCTION TO REMOVE WHEN THE REQUEST PARSING IS DONE !!!!*/
// std::string getRequestHost(std::string req)
// {
//     std::string::size_type pos = 0;
// 	std::string host;
//     while (true) {
//         std::string::size_type end = req.find('\n', pos);
//         if (end == std::string::npos) break;

//         std::string line = req.substr(pos, end - pos);
//         if (!line.empty() && line[line.size() - 1] == '\r')
//             line.erase(line.size() - 1);

//         if (line.size() >= 5 && line.substr(0, 5) == "Host:") {
// 			host = line.substr(6);
//             break;
//         }

//         pos = end + 1;
// 	}
// 	return host;
// }

//main HTTP handling function
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

	// Receive and parse header
	if (!request.HeaderComplete)
	{
		if (!request.ReceiveHeader())
		{
			std::cout << SOFT_RED "[ERROR] Failed to receive header" << RESET << std::endl;
			return;
		}
		if (request.HeaderComplete)
		{
			if (!request.ParseHeader() || !request.ValidateHeader())
				return; // Error logged in those functions
		}
		else
			return; // Header not complete yet
	}

	// Receive body if expected
	if (request.HeaderComplete && request.ExpectingBody && !request.BodyComplete && request.AnswerType != ERROR)
	{
		if (!request.ReceiveBody())
		{
			std::cout << SOFT_RED "[ERROR] Failed to receive body" << RESET << std::endl;
			return;
		}
		if (!request.BodyComplete)
			return; // Body not complete yet
	}
	
	// Check request validity
	if (request.AnswerType != ERROR)
		request.CheckRequest();

	// Generate response
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


