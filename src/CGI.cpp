#include "CGI.hpp"
#include "HttpRequest.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <sys/stat.h>

CGIHandler::CGIHandler() : _env_array(NULL), _env_count(0), _env_capacity(0)
{
}

CGIHandler::~CGIHandler()
{
	_cleanupEnvironment();
}

void	CGIHandler::_cleanupEnvironment()
{
	if (_env_array)
	{
		for (int i = 0; i < _env_count; ++i)
		{
			if (_env_array[i])
				delete[] _env_array[i];
		}
		delete[] _env_array;
		_env_array = NULL;
	}
	_env_count = 0;
	_env_capacity = 0;
}

char	*CGIHandler::_strdup(const std::string& str)
{
	char	*result = new char[str.length() + 1];
	std::strcpy(result, str.c_str());
	return (result);
}

std::string	CGIHandler::_toString(int value)
{
	std::string	result;
	bool		negative = false;

	if (value == 0)
		return ("0");
	if (value < 0)
	{
		negative = true;
		value = -value;
	}
	while (value > 0)
	{
		result = static_cast<char>('0' + (value % 10)) + result;
		value /= 10;
	}
	if (negative)
		result = "-" + result;
	return (result);
}

bool	CGIHandler::isCGI(const std::string& path, const LocationConfig& location)
{
	// Check if path starts with location path and has cgi_path configured
	if (!location.cgi_path.empty() && path.find(location.path) == 0)
		return (true);

	// Check for common CGI extensions
	if (path.find(".cgi") != std::string::npos ||
		path.find(".py") != std::string::npos ||
		path.find(".sh") != std::string::npos)
		return (true);

	return (false);
}

std::string	CGIHandler::_extractQueryString(const std::string& uri)
{
	size_t	pos = uri.find('?');
	if (pos != std::string::npos && pos + 1 < uri.length())
		return (uri.substr(pos + 1));
	return ("");
}

std::string	CGIHandler::_getScriptPath(const std::string& uri)
{
	size_t	pos = uri.find('?');
	if (pos != std::string::npos)
		return (uri.substr(0, pos));
	return (uri);
}

void	CGIHandler::_addEnvVar(const std::string& name, const std::string& value)
{
	// Expand array if needed
	if (_env_count >= _env_capacity)
	{
		int		new_capacity = _env_capacity == 0 ? 16 : _env_capacity * 2;
		char	**new_array = new char*[new_capacity];

		for (int i = 0; i < _env_count; ++i)
			new_array[i] = _env_array[i];
		if (_env_array)
			delete[] _env_array;
		_env_array = new_array;
		_env_capacity = new_capacity;
	}

	// Create name=value string
	std::string	env_string = name + "=" + value;
	_env_array[_env_count] = _strdup(env_string);
	++_env_count;
}

void	CGIHandler::_setupEnvironment(const RequestHeader& request,
								const LocationConfig& location,
								const std::string& script_path)
{
	(void)location; // Suppress unused parameter warning

	// Clean up previous environment
	_cleanupEnvironment();

	// Basic CGI environment variables
	std::map<std::string, std::string>	headers = request.getHeaders();

	_addEnvVar("REQUEST_METHOD", request.getMethod());
	_addEnvVar("REQUEST_URI", request.getUri());
	_addEnvVar("SCRIPT_NAME", script_path);
	_addEnvVar("QUERY_STRING", _extractQueryString(request.getUri()));

	// Server info
	std::map<std::string, std::string>::const_iterator	host_it;
	host_it = headers.find("Host");
	if (host_it != headers.end())
		_addEnvVar("HTTP_HOST", host_it->second);

	// Content info for POST requests
	if (request.getMethod() == "POST")
	{
		std::map<std::string, std::string>::const_iterator	ct_it, cl_it;
		ct_it = headers.find("Content-Type");
		if (ct_it != headers.end())
			_addEnvVar("CONTENT_TYPE", ct_it->second);

		cl_it = headers.find("Content-Length");
		if (cl_it != headers.end())
			_addEnvVar("CONTENT_LENGTH", cl_it->second);
	}

	// HTTP headers (convert to CGI format: HTTP_HEADER_NAME)
	std::map<std::string, std::string>::const_iterator	it;
	for (it = headers.begin(); it != headers.end(); ++it)
	{
		std::string	env_name = "HTTP_" + it->first;
		// Convert to uppercase and replace - with _
		for (size_t i = 0; i < env_name.size(); ++i)
		{
			if (env_name[i] == '-')
				env_name[i] = '_';
			else if (env_name[i] >= 'a' && env_name[i] <= 'z')
				env_name[i] = env_name[i] - 'a' + 'A';
		}
		_addEnvVar(env_name, it->second);
	}
}

std::string	CGIHandler::executeCGI(const std::string& script_path,
							const RequestHeader& request,
							const LocationConfig& location,
							const std::string& body)
{
	int	pipe_in[2], pipe_out[2];
	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
		return ("HTTP/1.1 500 Internal Server Error\r\n\r\nCGI pipe failed");

	// Setup environment
	_setupEnvironment(request, location, script_path);

	// Add NULL terminator to environment array
	char	**env_for_exec = new char*[_env_count + 1];
	for (int i = 0; i < _env_count; ++i)
		env_for_exec[i] = _env_array[i];
	env_for_exec[_env_count] = NULL;
	
	pid_t	pid = fork();
	if (pid == -1)
	{
		close(pipe_in[0]); close(pipe_in[1]);
		close(pipe_out[0]); close(pipe_out[1]);
		delete[] env_for_exec;
		return ("HTTP/1.1 500 Internal Server Error\r\n\r\nCGI fork failed");
	}

	if (pid == 0)
	{
		// Child process - execute CGI script
		close(pipe_in[1]);   // Close write end of input pipe
		close(pipe_out[0]);  // Close read end of output pipe

		// Redirect stdin and stdout
		if (dup2(pipe_in[0], STDIN_FILENO) == -1 ||
			dup2(pipe_out[1], STDOUT_FILENO) == -1)
		{
			delete[] env_for_exec;
			close(pipe_in[0]);
			close(pipe_out[1]);
			exit(1);
		}

		close(pipe_in[0]);
		close(pipe_out[1]);

		// Execute CGI script
		char	*argv[2];
		argv[0] = const_cast<char*>(script_path.c_str());
		argv[1] = NULL;

		execve(script_path.c_str(), argv, env_for_exec);
		// If execve returns, there was an error
		exit(1);
	}
	else
	{
		// Parent process
		close(pipe_in[0]);   // Close read end of input pipe
		close(pipe_out[1]);  // Close write end of output pipe
		delete[] env_for_exec;		
		// Write POST data to CGI process if any
		if (!body.empty())
			write(pipe_in[1], body.c_str(), body.length());
		close(pipe_in[1]);

		// Read output from CGI process
		std::string	output;
		char		buffer[1024];
		ssize_t		bytes_read;
		size_t		total_read = 0;

		while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[bytes_read] = '\0';
			output += buffer;
			total_read += bytes_read;
		}
		close(pipe_out[0]);
		
		int	status;
		waitpid(pid, &status, 0);

		// Check if CGI script executed successfully
		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		{
			// Parse CGI output - separate headers from body
			std::string	response;
			size_t		header_end = output.find("\n\n");
			
			if (header_end == std::string::npos)
			{
				// Try CRLF format
				header_end = output.find("\r\n\r\n");
			}
			
			if (header_end != std::string::npos)
			{
				// CGI script provided headers - parse them
				std::string	cgi_headers = output.substr(0, header_end);
				std::string	cgi_body;
				
				if (output.find("\r\n\r\n") != std::string::npos)
				{
					cgi_body = output.substr(header_end + 4); // Skip \r\n\r\n
				}
				else
				{
					cgi_body = output.substr(header_end + 2); // Skip \n\n
				}
				
				// Build proper HTTP response
				response = "HTTP/1.1 200 OK\r\n";
				response += cgi_headers;
				// Replace any \n with \r\n for HTTP compliance
				size_t pos = 0;
				while ((pos = response.find("\n", pos)) != std::string::npos)
				{
					if (pos == 0 || response[pos-1] != '\r')
					{
						response.insert(pos, "\r");
						pos += 2;
					}
					else
					{
						pos++;
					}
				}
				response += "\r\n\r\n" + cgi_body;
			}
			else
			{
				// No headers found, add default
				response = "HTTP/1.1 200 OK\r\n";
				response += "Content-Type: text/html\r\n";
				response += "\r\n" + output;
			}
			
			return (response);
		}
		else
		{
			return ("HTTP/1.1 500 Internal Server Error\r\n\r\nCGI execution failed");
		}
	}
}
