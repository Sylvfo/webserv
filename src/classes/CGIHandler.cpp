#include "Webserv.hpp"

CGIHandler::CGIHandler() {
	// Constructor implementation (if needed)
}
CGIHandler::~CGIHandler() {
	// Destructor implementation (if needed)
}

void CGIHandler::setEnv(const std::string& key, const std::string& value) {
	envVariables[key] = value;
}

void CGIHandler::setupCGIEnvironment(CGIHandler& cgi, const HttpRequest& request) {
    cgi.setEnv("REQUEST_METHOD", request.getMethod()); // GET, POST, etc.
    cgi.setEnv("SCRIPT_NAME", request.getScriptName()); // The path to the CGI script
    cgi.setEnv("QUERY_STRING", request.getQueryString());
    cgi.setEnv("SERVER_PROTOCOL", "HTTP/1.1");
    cgi.setEnv("SERVER_NAME", request.getServerName());
    cgi.setEnv("SERVER_PORT", request.getServerPort());
    cgi.setEnv("PATH_INFO", request.getPathInfo());
    cgi.setEnv("CONTENT_TYPE", request.getHeader("Content-Type"));
    cgi.setEnv("CONTENT_LENGTH", request.getHeader("Content-Length"));
    cgi.setEnv("GATEWAY_INTERFACE", "CGI/1.1");
    

    cgi.setEnv("HTTP_USER_AGENT", request.getHeader("User-Agent"));
    cgi.setEnv("HTTP_ACCEPT", request.getHeader("Accept"));
    cgi.setEnv("HTTP_COOKIE", request.getHeader("Cookie"));
}