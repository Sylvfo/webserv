#include "Webserv.hpp"

LocationConfig::LocationConfig()
	: path()
	, methods()
	, autoindex(false)
	, index()
	, root()
	, upload_path()
	, cgi_path()
	, returns()
{
}

LocationConfig::LocationConfig(const LocationConfig& other)
	: path(other.path)
	, methods(other.methods)
	, autoindex(other.autoindex)
	, index(other.index)
	, root(other.root)
	, upload_path(other.upload_path)
	, cgi_path(other.cgi_path)
	, returns(other.returns)
{
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other)
{
	if (this != &other)
	{
		path = other.path;
		methods = other.methods;
		autoindex = other.autoindex;
		index = other.index;
		root = other.root;
		upload_path = other.upload_path;
		cgi_path = other.cgi_path;
		returns = other.returns;
	}
	return *this;
}

LocationConfig::~LocationConfig()
{
}