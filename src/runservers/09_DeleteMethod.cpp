#include "Webserv.hpp"

void HttpRequest::DeleteRequest()
{
	std::cout << "DELETE in the workings " << std::endl;
	//to do...

	//get uri from server

	std::string uri = HTTPHeader.getUri();

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
		this->StatusCodeI = 404;
		std::cout << SOFT_RED "No location matched, using server defaults" << RESET << std::endl;
		return;
	}

	std::cout << "Matched location: " << Server->locations[matchedIndex].path << std::endl;
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
		this->StatusCodeI = 405;
		std::cout << SOFT_RED "DELETE method not allowed for this location" << RESET << std::endl;
		return;
	}

	std::string root;
	if (!Server->locations[matchedIndex].root.empty())
		root = Server->locations[matchedIndex].root;
	else
		root = Server->root; // maybe needs to error or print a warning because it is in root folder
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
		this->StatusCodeI = 403;
		std::cout << SOFT_RED "Cannot delete root directory" << RESET << std::endl;
		return;
	}

	std::cout << SOFT_GREEN "File to delete: " << RESET << filePath << std::endl;

	struct stat fileInfo;
	if (stat(filePath.c_str(), &fileInfo) != 0)
	{
		this->StatusCodeI = 404;
		std::cout << SOFT_RED "File not found: " << filePath << RESET << std::endl;
		return;
	}
	if (S_ISDIR(fileInfo.st_mode) || !(S_ISREG(fileInfo.st_mode)))
	{
		this->StatusCodeI = 403; // forbidden
		std::cout << SOFT_RED "Cannot delete directory or unregular files: " << filePath << RESET << std::endl;
		return;
	}
	std::cout << SOFT_GREEN "File exists and is not a directory or unregular file" << RESET << std::endl;


	// check write permission
	std::string parentDir = filePath.substr(0, filePath.find_last_of('/'));
	if (access(parentDir.c_str(), W_OK) != 0)
	{
		this->StatusCodeI = 403;
		std::cout << SOFT_RED "No write permission on directory: " << parentDir << RESET << std::endl;
		return;
	}
	if (std::remove(filePath.c_str()) == 0)
	{
		this->StatusCodeI = 204; // no content (successful deletion)
		std::cout << SOFT_GREEN "Successfully deleted: " << filePath << RESET << std::endl;
	}
	else
	{
		this->StatusCodeI = 500; // internal server error
		std::cout << SOFT_RED "Failed to delete: " << filePath << RESET << std::endl;
	}
}
