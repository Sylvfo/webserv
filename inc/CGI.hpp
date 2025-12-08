#ifndef CGI_HPP
#define CGI_HPP

#include <string>
#include <map>
#include <vector>
#include "ServerConfig.hpp"

// Forward declaration
class RequestHeader;

class CGIHandler
{
private:
	char	**_env_array;
	int		_env_count;
	int		_env_capacity;

public:
	CGIHandler();
	~CGIHandler();

	// Check if path should be handled by CGI
	bool	isCGI(const std::string& path, const LocationConfig& location);

	// Execute CGI script and return HTTP response
	std::string	executeCGI(const std::string& script_path,
						const RequestHeader& request,
						const LocationConfig& location,
						const std::string& body);

private:
	// Set up CGI environment variables (C++98 compliant)
	void	_setupEnvironment(const RequestHeader& request,
							const LocationConfig& location,
							const std::string& script_path);

	// Clean up environment array
	void	_cleanupEnvironment();

	// Helper to convert int to string (C++98 compatible)
	std::string	_toString(int value);

	// Extract query string from URI
	std::string	_extractQueryString(const std::string& uri);

	// Get script path without query string
	std::string	_getScriptPath(const std::string& uri);

	// Add environment variable to array
	void	_addEnvVar(const std::string& name, const std::string& value);

	// Create char* from string (for C++98 compatibility)
	char	*_strdup(const std::string& str);
};

#endif
