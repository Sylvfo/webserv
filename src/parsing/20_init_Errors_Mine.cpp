#include "Webserv.hpp"

void WebServ::initMimeTypes(struct ServerConfig &server) {
	server.mime_types[".html"] = "text/html";
	server.mime_types[".htm"]  = "text/html";
	server.mime_types[".css"]  = "text/css";
	server.mime_types[".js"]   = "application/javascript";
	server.mime_types[".json"] = "application/json";
	server.mime_types[".png"]  = "image/png";
	server.mime_types[".jpg"]  = "image/jpeg";
	server.mime_types[".jpeg"] = "image/jpeg";
	server.mime_types[".gif"]  = "image/gif";
	server.mime_types[".svg"]  = "image/svg+xml";
	server.mime_types[".ico"]  = "image/x-icon";
	server.mime_types[".txt"]  = "text/plain";
	server.mime_types[".pdf"]  = "application/pdf";
	server.mime_types["defaut"]  = "application/octet-stream";
}

void WebServ::initErroCode(struct ServerConfig &server)
{
	server.error_code_message.insert(std::make_pair(200, "200 OK"));
	server.error_code_message.insert(std::make_pair(400, "400 Bad Request"));
	server.error_code_message.insert(std::make_pair(403, "403 Forbidden"));
	server.error_code_message.insert(std::make_pair(404, "404 Not Found"));
	server.error_code_message.insert(std::make_pair(405, "405 Method Not Allowed"));
	server.error_code_message.insert(std::make_pair(500, "500 Internal Server Error"));
	server.error_code_message.insert(std::make_pair(501, "501 Not Implemented"));
	server.error_code_message.insert(std::make_pair(502, "502 Bad Gateway"));
	server.error_code_message.insert(std::make_pair(503, "503 Service Unavailable"));
}