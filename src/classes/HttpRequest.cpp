#include "Webserv.hpp"

static int findChar(const std::string& str, char c) {
	for (size_t i = 0; i < str.size(); i++) {
		if (str[i] == c)
			return i;
	}
	return -1;
}

static int findLastChar(const std::string& str, char c) {
	for (int i = str.size() - 1; i >= 0; i--) {
		if (str[i] == c)
			return i;
	}
	return -1;
}

static std::string substring(const std::string& str, size_t start, size_t len) {
	std::string result;
	for (size_t i = start; i < start + len && i < str.size(); i++) {
		result += str[i];
	}
	return result;
}

// Helper function to convert int to string manually
static std::string intToString(int num) {
	if (num == 0)
		return "0";
	
	std::string result;
	bool negative = false;
	
	if (num < 0) {
		negative = true;
		num = -num;
	}
	
	while (num > 0) {
		result = static_cast<char>('0' + (num % 10)) + result;
		num /= 10;
	}
	
	if (negative)
		result = "-" + result;
	
	return result;
}

std::string HttpRequest::getMethod() const {
	return method;
}

std::string HttpRequest::getScriptName() const {
	// Extract script name from URI (path before query string)
	int queryPos = findChar(uri, '?');
	if (queryPos != -1) {
		return substring(uri, 0, queryPos);
	}
	return uri;
}

std::string HttpRequest::getQueryString() const {
	// Extract query string from URI (everything after '?')
	int queryPos = findChar(uri, '?');
	if (queryPos != -1 && queryPos + 1 < (int)uri.size()) {
		return substring(uri, queryPos + 1, uri.size() - queryPos - 1);
	}
	return "";
}

std::string HttpRequest::getServerName() const {
	if (Server) {
		return Server->server_name;
	}
	// Fallback: try to get from Host header
	std::map<std::string, std::string>::const_iterator it = headers.find("Host");
	if (it != headers.end()) {
		return it->second;
	}
	return "";
}

std::string HttpRequest::getServerPort() const {
	if (Server) {
		return intToString(Server->listen_port);
	}
	return "80";
}

std::string HttpRequest::getPathInfo() const {
	std::string scriptName = getScriptName();
	
	int extPos = findLastChar(scriptName, '.');
	if (extPos != -1) {
		for (size_t i = extPos; i < scriptName.size(); i++) {
			if (scriptName[i] == '/') {
				return substring(scriptName, i, scriptName.size() - i);
			}
		}
	}
	return "";
}

std::string HttpRequest::getHeader(const std::string& key) const {
	// Search for the header in the headers map
	std::map<std::string, std::string>::const_iterator it = headers.find(key);
	if (it != headers.end()) {
		return it->second;
	}
	return "";
}