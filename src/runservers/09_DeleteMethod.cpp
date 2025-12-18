#include "Webserv.hpp"

void HttpRequest::deleteRequest()
{
	//get uri from server

	std::string uri = this->uri;

	// loop through locations to find longest match
	// finde longest match
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
	
	// check if delete is allowed
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
		root = Server->root; // maybe needs to error or print a warning because it is in root folder
	}
	const std::string& locationPath = Server->locations[matchedIndex].path;
	std::string relativePath = uri.substr(locationPath.length());
	std::string filePath;
	if (root[root.length() - 1] == '/')
		filePath = root + locationPath.substr(1) + relativePath;  // Remove leading / from locationPath
	else
		filePath = root + locationPath + relativePath;
	
	// Prevent deleting directory itself
	if (relativePath.empty() || relativePath == "/")
	{
		this->status_code = 403;
		return;
	}

	// Check if file exists first
	if (access(filePath.c_str(), F_OK) != 0)
	{
		this->status_code = 404;
		return;
	}
	
	// Check if we have read permission to stat the file
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
		this->status_code = 403; // forbidden
		return;
	}

	// check write permission
	std::string parentDir = filePath.substr(0, filePath.find_last_of('/'));
	if (access(parentDir.c_str(), W_OK) != 0)
	{
		this->status_code = 403;
		return;
	}
	
	if (std::remove(filePath.c_str()) == 0)
	{
		this->status_code = 204; // no content (successful deletion)
	}
	else
	{
		this->status_code = 500; // internal server error
	}
}
