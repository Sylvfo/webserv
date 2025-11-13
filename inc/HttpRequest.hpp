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

enum AcceptType {
	NONE,
	TXT,
	IMG,
};

struct server;

class RequestHeader
{
private:
	std::string method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	//std::string Host;
	std::string Accept;
	std::string Path;
public:
	RequestHeader();
	~RequestHeader();
	void    parseRequest(const std::string& request);
	void	parseHeaderRequest();//done by Syl
	std::map<std::string, std::string> getHeaders() const;
	void	printHeaders() const;
	std::string getUri();
	std::string getPath();
	std::string getMethod();
	std::string getAccept();
};

class HttpRequest{

	public:
	
	std::string RawRequest;
	RequestHeader HTTPHeader;
	std::string RequestBody;
	std::string HTTPAnswer;
	std::string AnswerBody;

	
	int	socket_fd;
	int AnswerType;
	int HttpMethod;
	ServerConfig	Server;//pointer?

	int errorCode;
	

	void setSocketFd(int fd);
	void linkServer(int index);
	void recieveRequest();
	void sendAnswerToRequest();
	void parseRequest();
	void checkRequest();
	void errortype();
	void Answerlocal();
	void AnswerCGI();

	void CreateHttpAnswerHead();

	void loadIndexPage();
	void loadPage();
};



std::string IntToString(int numb);

/*
bool parseHttpRequest(HttpRequest Request, server &ThisServer)
{

}*/