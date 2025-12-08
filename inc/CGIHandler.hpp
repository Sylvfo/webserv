#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include "Webserv.hpp"

class CGIHandler {

private:
	std::map<std::string, std::string> envVariables;

	void setEnv(const std::string& key, const std::string& value);

public:
	CGIHandler();
	~CGIHandler();

	void setupCGIEnvironment(CGIHandler& cgi, const HttpRequest& request);
	void printEnv() const;
	void clearEnv();
	void executeCGIScript(const std::string& scriptPath, const std::string& inputData, std::string& outputData);

};