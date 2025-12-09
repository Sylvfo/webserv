#include "Webserv.hpp"

//check request avec port et nom de domaine ici ou aprse?

/*TEMP FUNCTION TO REMOVE WHEN THE REQUEST PARSING IS DONE !!!!*/
std::string getRequestHost(std::string req)
{
    std::string::size_type pos = 0;
	std::string host;
    while (true) {
        std::string::size_type end = req.find('\n', pos);
        if (end == std::string::npos) break;

        std::string line = req.substr(pos, end - pos);
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        if (line.size() >= 5 && line.substr(0, 5) == "Host:") {
			host = line.substr(6);
            break;
        }

        pos = end + 1;
	}
	return host;
}

//main HTTP handling function
void WebServ::handleRequest(epoll_event current_event)
{
	HttpRequest	Request;

	//std::cout << "enter handlerequest" << std::endl;
	//this can be put in a function called takeInfofrom server Pointer
	ConnectionData* connInfo = static_cast<ConnectionData*>(current_event.data.ptr);
   	if (!connInfo)
    {
        std::cerr << "Error: NULL connection info" << std::endl;
        return;
    }
	Request.socket_fd = connInfo->client_fd;
	Request.Server = connInfo->server;
	//////////////////////////////////////////////////////////
	//PARSING TO DO BETTER
	if (Request.recieveRequest() == false)
		return;//to do better

	Request.parseRequest(); //to do 
	//Request.printHttpRequest();
	
	Request.method = Request.HTTPHeader.getMethod();// pas bien à refaire. 
	Request.uri = Request.HTTPHeader.getUri();
	Request.checkRequest(); //to do
	Request.AnswerType = STATIC; 
	//Request.AnswerType = ERROR;// LOCAL; //to remove when parsing is done
	//Request.StatusCode = 403;
	
	if (Request.RawRequest.empty() || Request.HTTPHeader.getMethod().empty())
    {
		// todoparsing why they are empty request????
    //    std::cout << "Empty request, closing connection" << std::endl;
        return;
    }
	////////////////////////////////////////////////////////////
	//CREATING THE ANSWER
	if (Request.AnswerType == ERROR)
		Request.AnswerError();
	else if (Request.AnswerType == STATIC)
		Request.Answerlocal();
	else if (Request.AnswerType == CGI)
	{
		std::cout << "[CGI] Processing CGI request for: " << Request.uri << std::endl;
		Request.AnswerCGI();//to do
	}
	Request.sendAnswerToRequest();
}

void HttpRequest::sendAnswerToRequest()
{
	int bytesSent = 0;
	int totalBytesSent = 0;

	//std::cout << "socket fd " << socket_fd << std::endl;
	// is it non blocking?
	while(totalBytesSent < (int)HttpAnswer.size())
	{
		bytesSent = send(socket_fd, HttpAnswer.c_str(), HttpAnswer.size(), 0);
		if (bytesSent < 0)
		{
			std::cout << "Could not send response";
			return; 
		}
		totalBytesSent += bytesSent;
	}
	//epoll_ctl(epollFd, EPOLL_CTL_DEL, socket_fd, NULL);
//	close(socket_fd);
}

std::string IntToString(int numb)
{
	std::string value;
	std::stringstream out;

	out << numb;
	value = out.str();
	return (value);
}


