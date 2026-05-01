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

CGIHandler::CGIHandler() : _env_array(NULL), _env_count(0), _env_capacity(0)
{
}

CGIHandler::CGIHandler(const CGIHandler& other)
	: _env_array(NULL), _env_count(0), _env_capacity(0)
{
	*this = other;
}

CGIHandler& CGIHandler::operator=(const CGIHandler& other)
{
	if (this != &other)
	{
		_cleanupEnvironment();
		_env_capacity = other._env_capacity;
		_env_count = other._env_count;
		
		if (other._env_array && other._env_count > 0)
		{
			_env_array = new char*[_env_capacity];
			for (int i = 0; i < _env_count; ++i)
			{
				if (other._env_array[i])
					_env_array[i] = _strdup(std::string(other._env_array[i]));
				else
					_env_array[i] = NULL;
			}
		}
	}
	return *this;
}

CGIHandler::~CGIHandler()
{
	_cleanupEnvironment();
}