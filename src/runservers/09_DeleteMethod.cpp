#include "Webserv.hpp"

void HttpRequest::deleteRequest()
{
	std::string uri = this->uri;
	size_t longestMatch = 0;
	int matchedIndex = -1;

	for (size_t i = 0; i < Server->locations.size(); i++)
	{
		const std::string& locationPath = Server->locations[i].path;
		if (uri.find(locationPath) == 0)
		{
			if (locationPath.length() > longestMatch)
			{
				longestMatch = locationPath.length();
				matchedIndex = i;
			}
		}
	}

	if (matchedIndex == -1)
	{
		this->status_code = 404;
		return;
	}

	bool deleteAllowed = false;
	for (size_t j = 0; j < Server->locations[matchedIndex].methods.size(); j++)
	{
		if (Server->locations[matchedIndex].methods[j] == "DELETE")
		{
			deleteAllowed = true;
			break;
		}
	}
	if (!deleteAllowed)
	{
		this->status_code = 405;
		return;
	}

	std::string root;
	if (!Server->locations[matchedIndex].root.empty())
	{
		root = Server->locations[matchedIndex].root;
	}
	else
	{
		root = Server->root;
	}
	const std::string& locationPath = Server->locations[matchedIndex].path;
	std::string relativePath = uri.substr(locationPath.length());
	std::string filePath;
	if (root[root.length() - 1] == '/')
		filePath = root + locationPath.substr(1) + relativePath;
	else
		filePath = root + locationPath + relativePath;

	// Prevent deleting directory itself
	if (relativePath.empty() || relativePath == "/")
	{
		this->status_code = 403;
		return;
	}

	if (access(filePath.c_str(), F_OK) != 0)
	{
		this->status_code = 404;
		return;
	}

	if (access(filePath.c_str(), R_OK) != 0)
	{
		this->status_code = 403;
		return;
	}

	struct stat fileInfo;
	if (stat(filePath.c_str(), &fileInfo) != 0)
	{
		this->status_code = 500;
		return;
	}
	if (S_ISDIR(fileInfo.st_mode) || !(S_ISREG(fileInfo.st_mode)))
	{
		this->status_code = 403;
		return;
	}

	std::string parentDir = filePath.substr(0, filePath.find_last_of('/'));
	if (access(parentDir.c_str(), W_OK) != 0)
	{
		this->status_code = 403;
		return;
	}

	if (std::remove(filePath.c_str()) == 0)
	{
		this->status_code = 204;
	}
	else
	{
		this->status_code = 500;
	}
}
