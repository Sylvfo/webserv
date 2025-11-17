

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "Webserv.hpp"
/*
enum HttpMethod {
	GET_METHOD,
	POST_METHOD,
	DELETE_METHOD // renamed to avoid conflict with C++ keyword
};*/

enum AnswerType {
	ERROR,
	LOCAL,//find a better name??
	CGI,
};

/*
enum AcceptType {
	NONE,
	TXT,
	IMG,
};*/

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

class HttpRequest
{

	public:
	std::map<std::string, std::string> mimeTypes; //à déplacer
	void initMimeTypes();//à déplacer
	//maps....
	std::string RawRequest;
	RequestHeader HTTPHeader; //RequestHeader a effacer après new parsing. 
	std::string RequestBody;

	//Request
	std::string method;// a double pour l instant
	std::string uri;// a double pour l instant
	std::string version;// a double pour l instant
	std::map<std::string, std::string> headers;// a double pour l instant
//	std::string Accept;//apparement pas demandé dans webserv... (gpt)
//	std::string Path;//nécessaire??

	int AnswerType;

	//Response
	std::string StatusCode; //int?? pas besoin je pense pc on peut mettre direct dans HttpAnswer
	std::string HttpAnswer; //=)
	std::string ContentLenght; //=)
	std::string ContentType;
	
	int fd_Ressource;
	bool loadRessource();

	void SetStatusLine();
	void SetResponseHeader();
	void SetContentType(std::string &makingPath);
	//void loadResponseBody(); //ancien load page
	std::string AnswerBody;//row... a voir...

	
//	int AnswerType;
//	int HttpMethod;
	ServerConfig	Server;//pointer?
	int	socket_fd;
//	int errorCode;
	
	//03_handle_request
	void setSocketFd(int fd);
	void linkServer(int index);
	void recieveRequest();
	void sendAnswerToRequest();
	void parseRequest();
	void checkRequest();
	void errortype();
	void Answerlocal();
	void AnswerCGI();

	void GetRequest();
	bool GetAccessRessource();

	void PostRequest();
	void DeleteRequest();
	void printHttpRequest();
};

#endif



std::string IntToString(int numb);

/*
bool parseHttpRequest(HttpRequest Request, server &ThisServer)
{

}*/

/*

The HTTP protocol is based on a request/response paradigm. A client
establishes a connection with a server and sends a request to the
server in the form of a request method, URI, and protocol version,
followed by a MIME-like message containing request modifiers, client
information, and possible body content.

The server responds with a status line, including the message's protocol version and a success
or error code, followed by a MIME-like message containing server
information, entity metainformation, and possible body content.

On the Internet, HTTP communication generally takes place over TCP/IP
connections. The default port is TCP 80 [15], but other ports can be
used.

Both clients and
servers should be aware that either party may close the connection
prematurely, due to user action, automated time-out, or program
failure, and should handle such closing in a predictable fashion.

MIME are for mails. 

Uniform Resource Identifiers are
simply formatted strings which identify--via name, location, or any
other characteristic--a network resource

We have to work only on headers right?

Content-Type = media type

Full Request:
Request-Line = Method space Request-URI space HTTP-Version CRLF:)
*(General-Header
Request-Header //fields see p 25 is necessary?
Entity-Header)
CRLF???
[ Entity-Body ]

Body

Full Response:
Status Line (ResponseStatusCode  200 ok Error 404 not found...
Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF see p 27
*(General Header
Response Header
Entity Header)
CRLF???
[Entity body]

Head


Body

*/