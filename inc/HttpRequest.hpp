#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "ServerConfig.hpp"
#include "CGI.hpp"


enum answer_type {
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
	// Cononical Form
	// ==============================================================================================
	HttpRequest();
	HttpRequest(const HttpRequest& other);
	HttpRequest& operator=(const HttpRequest& other);
	~HttpRequest();

	// ==============================================================================================
	// Public Data Members
	// ==============================================================================================

	// --- Server & Connection ---
	ServerConfig*						Server;
	int									socket_fd;

	// --- Request Data ---
	std::string							method;
	std::string							uri;
	std::string							version;
	std::string							path;
	std::map<std::string, std::string>	headers;

	// --- Request State & Buffers ---
	bool			header_complete;
	bool			request_complete;
	bool 			is_directory;
	std::string		partial_request;
	std::string		raw_header;
	std::string		redirection_url;

	// --- Body Handling ---
	bool			expecting_body;
	bool			is_chunked;
	bool			body_complete;
	size_t			content_length;
	std::string 	raw_body;
	std::string		partial_body;

	// --- Response ---
	int				answer_type;
	int				status_code;
	int				fd_ressource;
	std::string		http_answer;
	std::string		content_type;
	std::string		answer_body;

	// ==============================================================================================
	// Public Interface
	// ==============================================================================================


	// --- Parsing & Validation ---
	bool	receiveHeader();
	bool	parseHeader();
	bool	validateHeader();
	void	checkRequest();
	bool	receiveBody();

	// --- Method Processing ---
	void	getRequest();
	void	postRequest();
	void	deleteRequest();

	// --- Response Generation ---
	void	answerLocal();
	void	answerCGI();
	void	answerError();
	void	sendAnswerToRequest();

	// --- Error Handling ---
	std::string	getCustomErrorPage();
	void		useDefaultErrorHTML();

private:
	// ==============================================================================================
	// Private Implementation Helpers
	// ==============================================================================================

	// --- Parsing Helpers ---
	bool	_parseRequestLine(const std::string&);
	bool	_parseOneHeader(const std::string&);
	void	_handleMultipart();

	// --- Response Helpers ---
	bool	_getAccessRessource();
	bool	_loadRessource();
	std::string	_generate_directory_listing(const std::string &dir_path, const std::string &uri_path);
	void	_setStatusLine();
	void	_setResponseHeader();
	void	_setcontent_type(std::string &makingPath);

	// --- Utilities ---
	void								_handleFormData();
	std::map<std::string, std::string>	_parseFormData(const std::string &body);
	std::string							_urlDecode(const std::string &str);
	std::string							_getCurrentTimestamp();
};

// --- Global Helper ---
std::string intToString(int numb);

#endif
