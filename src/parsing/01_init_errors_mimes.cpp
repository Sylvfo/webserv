#include "Webserv.hpp"

void WebServ::initMimeTypes(ServerConfig &server) {
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

void WebServ::initErroCode(ServerConfig &server)
{
	server.error_code_message.insert(std::make_pair(200, "200 OK"));
	server.error_code_message.insert(std::make_pair(301, "301 Moved Permanently"));
	server.error_code_message.insert(std::make_pair(400, "400 Bad Request"));
	server.error_code_message.insert(std::make_pair(403, "403 Forbidden"));
	server.error_code_message.insert(std::make_pair(404, "404 Not Found"));
	server.error_code_message.insert(std::make_pair(405, "405 Method Not Allowed"));
	server.error_code_message.insert(std::make_pair(413, "411 Length Required"));
	server.error_code_message.insert(std::make_pair(413, "413 Payload Too Large"));
	server.error_code_message.insert(std::make_pair(414, "414 URI Too Long"));
	server.error_code_message.insert(std::make_pair(500, "500 Internal Server Error"));
	server.error_code_message.insert(std::make_pair(501, "501 Not Implemented"));
	server.error_code_message.insert(std::make_pair(502, "502 Bad Gateway"));
	server.error_code_message.insert(std::make_pair(503, "503 Service Unavailable"));
}

void WebServ::initDefautlPage(ServerConfig &server)
{
	server.default_error_html.insert(std::make_pair(400, "<html><head><title>400 Bad Request</title></head><body><center><h1>400 Bad Request</h1></center><hr><center>Webserv</center></body></html>"));
	server.default_error_html.insert(std::make_pair(403, "<html><head><title>403 Forbidden</title></head><body><center><h1>403 Forbidden</h1></center><hr><center>Webserv</center></body></html>"));
	server.default_error_html.insert(std::make_pair(404, "<html><head><title>404 Not Found</title></head><body><center><h1>404 Not Found</h1></center><hr><center>Webserv</center></body></html>"));
	server.default_error_html.insert(std::make_pair(405, "<html><head><title>405 Method Not Allowed</title></head><body><center><h1>405 Method Not Allowed</h1></center><hr><center>Webserv</center></body></html>"));
	server.default_error_html.insert(std::make_pair(413, "<html><head><title>413 Payload Too Large</title></head><body><center><h1>413 Payload Too Large</h1></center><hr><center>Webserv</center></body></html>"));
	server.default_error_html.insert(std::make_pair(414, "<html><head><title>414 URI Too Long</title></head><body><center><h1>414 URI Too Long</h1></center><hr><center>Webserv</center></body></html>"));
	server.default_error_html.insert(std::make_pair(500, "<html><head><title>500 Internal Server Error</title></head><body><center><h1>500 Internal Server Error</h1></center><hr><center>Webserv</center></body></html>"));
	server.default_error_html.insert(std::make_pair(501, "<html><head><title>501 Not Implemented</title></head><body><center><h1>501 Not Implemented</h1></center><hr><center>Webserv</center></body></html>"));
	server.default_error_html.insert(std::make_pair(502, "<html><head><title>502 Bad Gateway</title></head><body><center><h1>502 Bad Gateway</h1></center><hr><center>Webserv</center></body></html>"));
	server.default_error_html.insert(std::make_pair(503, "<html><head><title>503 Service Unavailable</title></head><body><center><h1>503 Service Unavailable</h1></center><hr><center>Webserv</center></body></html>"));
}