#include "Webserv.hpp"
#include "CGI.hpp"
#include "HttpRequest.hpp"
#include "colors.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <signal.h>

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
	for (size_t i = 0; i < str.length(); ++i)
		result[i] = str[i];
	result[str.length()] = '\0';
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
	if (!location.cgi_path.empty() && path.find(location.path) == 0)
		return (true);

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

void CGIHandler::_addEnvVar(const std::string& name, const std::string& value)
{
	std::string env_string = name + "=" + value;
	char **new_array = new char*[_env_count + 1];

	for (int i = 0; i < _env_count; ++i)
		new_array[i] = _env_array[i];
	new_array[_env_count] = _strdup(env_string);
	if (_env_array)
		delete[] _env_array;
	_env_array = new_array;
	++_env_count;
	_env_capacity = _env_count;
}

void	CGIHandler::_setupEnvironment(const HttpRequest& request,
	const LocationConfig& location, const std::string& script_path)
{
	(void)location;
	_cleanupEnvironment();

	const std::map<std::string, std::string>&	headers = request.headers;

	_addEnvVar("REQUEST_METHOD", request.method);
	_addEnvVar("REQUEST_URI", request.uri);
	_addEnvVar("SCRIPT_NAME", script_path);
	_addEnvVar("SCRIPT_FILENAME", script_path);
	_addEnvVar("QUERY_STRING", _extractQueryString(request.uri));
	_addEnvVar("SERVER_PROTOCOL", request.version.empty() ? "HTTP/1.0" : request.version);
	_addEnvVar("GATEWAY_INTERFACE", "CGI/1.1");
	
	size_t cgi_pos = script_path.find("/cgi-bin/");
	if (cgi_pos != std::string::npos)
	{
		std::string doc_root = script_path.substr(0, cgi_pos);
		_addEnvVar("DOCUMENT_ROOT", doc_root);
	}
	else
	{
		_addEnvVar("DOCUMENT_ROOT", "www");
	}

	std::map<std::string, std::string>::const_iterator	host_it;
	host_it = headers.find("host");
	if (host_it != headers.end())
		_addEnvVar("HTTP_HOST", host_it->second);

	if (request.method == "POST")
	{
		std::map<std::string, std::string>::const_iterator	ct_it, cl_it;
		ct_it = headers.find("content-type");
		if (ct_it != headers.end())
			_addEnvVar("CONTENT_TYPE", ct_it->second);

		cl_it = headers.find("content-length");
		if (cl_it != headers.end())
			_addEnvVar("CONTENT_LENGTH", cl_it->second);
	}
		std::map<std::string, std::string>::const_iterator	it;
	for (it = headers.begin(); it != headers.end(); ++it)
	{
		std::string	env_name = "HTTP_" + it->first;
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

std::string	CGIHandler::executeCGI(const std::string& script_path, const HttpRequest& request, const LocationConfig& location, const std::string& body)
{
	struct stat script_stat;
	if (stat(script_path.c_str(), &script_stat) != 0)
	{
		return ("HTTP/1.1 404 Not Found\r\n\r\nCGI script not found");
	}
	
	if (access(script_path.c_str(), X_OK) != 0)
	{
		return ("HTTP/1.1 403 Forbidden\r\n\r\nCGI script not executable");
	}
	
	int	pipe_in[2];
	int	pipe_out[2];

	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
	{
		if (pipe_in[0] >= 0)
		{
			close(pipe_in[0]);
			close(pipe_in[1]);
		}
		return ("HTTP/1.1 500 Internal Server Error\r\n\r\nCGI pipe failed");
	}
	
	_setupEnvironment(request, location, script_path);

	char	**env_for_exec = new char*[_env_count + 1];
	for (int i = 0; i < _env_count; ++i)
		env_for_exec[i] = _env_array[i];
	env_for_exec[_env_count] = NULL;

	pid_t	pid = fork();
	if (pid == -1)
	{
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		delete[] env_for_exec;
		return ("HTTP/1.1 500 Internal Server Error\r\n\r\nCGI fork failed");
	}

	if (pid == 0)
	{
		close(pipe_in[1]);
		close(pipe_out[0]);

		if (dup2(pipe_in[0], STDIN_FILENO) == -1
			|| dup2(pipe_out[1], STDOUT_FILENO) == -1)
		{
			close(pipe_in[0]);
			close(pipe_out[1]);
			while (1)
				;
		}

		close(pipe_in[0]);
		close(pipe_out[1]);

		char	*argv[2];
		argv[0] = const_cast<char*>(script_path.c_str());
		argv[1] = NULL;

		execve(script_path.c_str(), argv, env_for_exec);
		while (1)
			;
	}
	else
	{
		close(pipe_in[0]);
		close(pipe_out[1]);
		delete[] env_for_exec;
		if (!body.empty())
		{
			ssize_t written = write(pipe_in[1], body.c_str(), body.length());
			(void)written;
		}
		close(pipe_in[1]);
		std::string	output;
		char		buffer[1024];
		ssize_t		bytes_read;

		while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[bytes_read] = '\0';
			output += buffer;
		}
		close(pipe_out[0]);
		int	status;
		waitpid(pid, &status, 0);

		if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		{
			std::string	response;
			size_t		header_end = output.find("\n\n");

			if (header_end == std::string::npos)
				header_end = output.find("\r\n\r\n");

			if (header_end != std::string::npos)
			{
				std::string	cgi_headers = output.substr(0, header_end);
				std::string	cgi_body;

				if (output.find("\r\n\r\n") != std::string::npos)
					cgi_body = output.substr(header_end + 4);
				else
					cgi_body = output.substr(header_end + 2);

				response = "HTTP/1.1 200 OK\r\n";
				response += cgi_headers;
				size_t pos = 0;
				while ((pos = response.find("\n", pos)) != std::string::npos)
				{
					if (pos == 0 || response[pos - 1] != '\r')
					{
						response.insert(pos, "\r");
						pos += 2;
					}
					else
						pos++;
				}
				response += "\r\n\r\n" + cgi_body;
			}
			else
			{
				response = "HTTP/1.1 200 OK\r\n";
				response += "Content-Type: text/html\r\n";
				response += "\r\n" + output;
			}
			return (response);
		}
		return ("HTTP/1.1 500 Internal Server Error\r\n\r\nCGI execution failed");
	}
	return ("HTTP/1.1 500 Internal Server Error\r\n\r\nCGI execution failed");
}

void HttpRequest::answerCGI()
{
	LocationConfig* matchingLocation = NULL;
	size_t bestMatchLength = 0;

	for (size_t i = 0; i < Server->locations.size(); ++i)
	{
		const std::string& locationPath = Server->locations[i].path;
		if (uri.find(locationPath) == 0 && locationPath.length() > bestMatchLength)
		{
			matchingLocation = &Server->locations[i];
			bestMatchLength = locationPath.length();
		}
	}
	
	if (!matchingLocation)
	{
		http_answer = "HTTP/1.1 404 Not Found\r\n\r\nLocation not found";
		return;
	}

	CGIHandler cgiHandler;
	std::string cleanUri = uri;
	size_t queryPos = cleanUri.find('?');
	if (queryPos != std::string::npos)
	{
		cleanUri = cleanUri.substr(0, queryPos);
	}
	
	std::string scriptPath;
	if (!matchingLocation->root.empty())
	{
		scriptPath = matchingLocation->root;
		if (scriptPath[scriptPath.length() - 1] != '/')
			scriptPath += "/";

		std::string relativePath = cleanUri.substr(matchingLocation->path.length());
		if (!relativePath.empty() && relativePath[0] == '/')
			relativePath = relativePath.substr(1);
		scriptPath += relativePath;
	}
	else
	{
		scriptPath = Server->root;
		if (scriptPath[scriptPath.length() - 1] != '/')
			scriptPath += "/";
		scriptPath += cleanUri.substr(1);
	}	

	http_answer = cgiHandler.executeCGI(scriptPath, *this, *matchingLocation, raw_body);
}