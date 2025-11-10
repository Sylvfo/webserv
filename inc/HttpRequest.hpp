#include "Webserv.hpp"


enum HttpMethod {
	GET_METHOD,
	POST_METHOD,
	DELETE_METHOD // renamed to avoid conflict with C++ keyword
};

enum AnswerType {
	ERROR,
	LOCAL,//find a better name??
	CGI,
};


struct server;

class HttpRequest{

	public:
	
	std::string Request;//usefull??
	std::string RequestHead;
	std::string RequestBody;
	std::string AnswerHead;
	std::string AnswerBody;
	std::string HTTPAnswer;
	int	socket_fd;
	int AnswerType;
	int HttpMethod;

	void setSocketFd(int fd);
	void linkServer(int index);
	void recieveRequest();
	void sendAnswerToRequest();
	void parseRequest();
	void checkRequest();
	void errortype();
	void Answerlocal();
	void AnswerCGI();
};

std::string IntToString(int numb);

/*
bool parseHttpRequest(HttpRequest Request, server &ThisServer)
{

}*/