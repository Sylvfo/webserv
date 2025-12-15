#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <map>
#include <vector>
#include "ServerConfig.hpp"

class HttpRequest;

class CGIHandler
{
private:
	char	**_env_array;
	int		_env_count;
	int		_env_capacity;

public:
	CGIHandler();
	~CGIHandler();

	bool	isCGI(const std::string& path, const LocationConfig& location);
	std::string	executeCGI(const std::string& script_path, const HttpRequest& request, const LocationConfig& location, const std::string& body);
private:
	void	_setupEnvironment(const HttpRequest& request, const LocationConfig& location, const std::string& script_path);
	void	_cleanupEnvironment();
	std::string	_toString(int value);
	std::string	_extractQueryString(const std::string& uri);
	std::string	_getScriptPath(const std::string& uri);
	void	_addEnvVar(const std::string& name, const std::string& value);
	char	*_strdup(const std::string& str);
};

#endif
