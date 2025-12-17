#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "ServerConfig.hpp"

enum AnswerType {
	ERROR,
	STATIC,//ancien Local
	CGI,
};

struct server;

class HttpRequest
{
public:
	// ==============================================================================================
	// Constants
	// ==============================================================================================
	static const size_t MAX_HEADER_SIZE = 8192;      // 8KB - nginx standard
	static const size_t RECEIVE_CHUNK_SIZE = 1024;   // 1KB receive buffer

	// ==============================================================================================
	// Constructor & Destructor
	// ==============================================================================================
	HttpRequest();
	HttpRequest(const HttpRequest& other);
	HttpRequest& operator=(const HttpRequest& other);
	~HttpRequest();

	// ==============================================================================================
	// Data Members
	// ==============================================================================================

	// --- Server & Connection ---
	ServerConfig*	Server;
	int				socket_fd;

	// --- Request Line & Headers ---
	std::string							method;
	std::string							uri;
	std::string							version;
	std::string							Path;
	std::map<std::string, std::string>	headers;

	// --- Request State & Buffers ---
	bool			HeaderComplete;
	std::string		PartialRequest;
	std::string		RawHeader;
	bool			RequestComplete;  // Flag to indicate if request is fully processed
	bool 			IsDirectory;
	std::string		RedirectionUrl;

	// --- Body Handling ---
	bool			ExpectingBody;
	bool			IsChunked;
	bool			BodyComplete;
	size_t			ContentLength;
	std::string 	RawBody;
	std::string		PartialBody;

	// --- Response ---
	int				AnswerType;
	int				StatusCode;
	std::string		HttpAnswer;
	std::string		ContentType;
	std::string		AnswerBody;
	int				fd_Ressource;

	// ==============================================================================================
	// Methods
	// ==============================================================================================

	// --- Parsing & Validation ---
	bool	ReceiveHeader();
	bool	ParseHeader();
	bool	ParseRequestLine(const std::string&);
	bool	ParseOneHeader(const std::string&);
	bool	ValidateHeader();
	void	CheckRequest();

	// --- Body Handling ---
	bool	ReceiveBody();
	void	HandleMultipart();

	// --- Response Generation ---
	void	Answerlocal();
	void	AnswerCGI();
	void	AnswerError();
	void	sendAnswerToRequest();

	// --- Method Handlers ---
	void	GetRequest();
	void	PostRequest();
	void	DeleteRequest();

	// --- Response Helpers ---
	bool	GetAccessRessource();
	bool	loadRessource();
	std::string	generate_directory_listing(const std::string &dir_path, const std::string &uri_path);
	void	SetStatusLine();
	void	SetResponseHeader();
	void	SetContentType(std::string &makingPath);

	// --- Utilities ---
	void								HandleFormData();
	std::map<std::string, std::string>	parseFormData(const std::string &body);
	std::string							urlDecode(const std::string &str);
	std::string							getCurrentTimestamp();

	// --- Error Handling ---
	std::string	GetCustomErrorPage();
	void		UseDefaultErrorHTML();
};

#endif

std::string IntToString(int numb);




