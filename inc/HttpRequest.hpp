#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "Webserv.hpp"

enum AnswerType {
	ERROR,
	LOCAL,//find a better name??
	CGI,
};

struct server;

// todoparsing : erase this class and let these info only in HttpRequest class
class RequestHeader
{
private:
	std::string method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string Accept;
	std::string Path;
public:
	RequestHeader();
	~RequestHeader();
	void    parseRequest(const std::string& request);
	void	parseHeaderRequest();//done by Syl
	std::map<std::string, std::string> getHeaders() const;
	void	printHeaders() const;
	std::string getUri() const;
	std::string getPath() const;
	std::string getMethod() const;
	std::string getAccept() const;
};

// todoparsing : put things in private + getter / setter
class HttpRequest
{
	public:
	std::map<std::string, std::string> mimeTypes; //à déplacer
	void initMimeTypes();//à déplacer

	RequestHeader	HTTPHeader; //RequestHeader a effacer après new parsing.

	//Request
	std::string		RawRequest;
	std::string		method;// a double pour l instant
	std::string		uri;// a double pour l instant
	std::string		version;// a double pour l instant
	std::map<std::string, std::string> headers;// a double pour l instant
	std::string RequestBody;
	ServerConfig	*Server;//pointer?
	int	socket_fd;

	//Response
	int			AnswerType;
	std::string StatusCode; //pas besoin je pense pc on peut mettre direct dans HttpAnswer
	std::string HttpAnswer; //=)
	std::string ContentLenght; //=)
	std::string ContentType;
	std::string AnswerBody;//row... a voir...
	int			fd_Ressource;

	//recieve request
	void recieveRequest();
	void parseRequest();
	void extractRequestBody(); //necessary to implement POST
	void checkRequest();

	//answer request
	void Answerlocal();
	void AnswerCGI();
	void errortype();
	//GET
	void GetRequest();
	bool GetAccessRessource();
	bool loadRessource();
	void SetStatusLine();
	void SetResponseHeader();
	void SetContentType(std::string &makingPath);
	void sendAnswerToRequest();
	//POST
	void PostRequest();
	void HandleFormData();
	std::map<std::string, std::string> parseFormData(const std::string &body);
	std::string urlDecode(const std::string &str);
	std::string getCurrentTimestamp();
	//DELETE
	void DeleteRequest();

	void printHttpRequest();

	// CGI-related getter methods
	std::string getMethod() const;
	std::string getScriptName() const;
	std::string getQueryString() const;
	std::string getServerName() const;
	std::string getServerPort() const;
	std::string getPathInfo() const;
	std::string getHeader(const std::string& key) const;

};

#endif

std::string IntToString(int numb);

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
