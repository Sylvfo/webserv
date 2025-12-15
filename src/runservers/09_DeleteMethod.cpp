#include "Webserv.hpp"

void HttpRequest::DeleteRequest()
{
	std::cout << SOFT_ORANGE "[DELETE] Processing DELETE request for URI: " << this->uri << RESET << std::endl;

	//get uri from server

	std::string uri = this->uri;

	// loop through locations to find longest match
	// finde longest match
	size_t longestMatch = 0;
	int matchedIndex = -1;

	std::cout << SOFT_ORANGE "[DELETE] Searching for matching location..." << RESET << std::endl;
	for (size_t i = 0; i < Server->locations.size(); i++)
	{
		const std::string& locationPath = Server->locations[i].path;
		if (uri.find(locationPath) == 0)
		{
			if (locationPath.length() > longestMatch)
			{
				longestMatch = locationPath.length();
				matchedIndex = i;
				std::cout << SOFT_ORANGE "[DELETE] Found match: " << locationPath << " (length: " << locationPath.length() << ")" << RESET << std::endl;
			}
		}
	}

	if (matchedIndex == -1)
	{
		this->StatusCode = 404;
		std::cout << SOFT_RED "[DELETE] No location matched, returning 404" << RESET << std::endl;
		return;
	}

	std::cout << SOFT_ORANGE "[DELETE] Best match: " << Server->locations[matchedIndex].path << RESET << std::endl;
	
	// check if delete is allowed
	std::cout << SOFT_ORANGE "[DELETE] Checking if DELETE method is allowed..." << RESET << std::endl;
	bool deleteAllowed = false;
	for (size_t j = 0; j < Server->locations[matchedIndex].methods.size(); j++)
	{
		if (Server->locations[matchedIndex].methods[j] == "DELETE")
		{
			deleteAllowed = true;
			std::cout << SOFT_ORANGE "[DELETE] DELETE method is allowed" << RESET << std::endl;
			break;
		}
	}
	if (!deleteAllowed)
	{
		this->StatusCode = 405;
		std::cout << SOFT_RED "[DELETE] DELETE method not allowed for this location (405)" << RESET << std::endl;
		return;
	}

	std::string root;
	if (!Server->locations[matchedIndex].root.empty())
	{
		root = Server->locations[matchedIndex].root;
		std::cout << SOFT_ORANGE "[DELETE] Using location root: " << root << RESET << std::endl;
	}
	else
	{
		root = Server->root; // maybe needs to error or print a warning because it is in root folder
		std::cout << SOFT_ORANGE "[DELETE] Using server root: " << root << RESET << std::endl;
	}
	const std::string& locationPath = Server->locations[matchedIndex].path;
	std::string relativePath = uri.substr(locationPath.length());
	std::string filePath;
	if (root[root.length() - 1] == '/')
		filePath = root + locationPath.substr(1) + relativePath;  // Remove leading / from locationPath
	else
		filePath = root + locationPath + relativePath;
	
	std::cout << SOFT_ORANGE "[DELETE] Constructed file path: " << filePath << RESET << std::endl;
	
	// Prevent deleting directory itself
	if (relativePath.empty() || relativePath == "/")
	{
		this->StatusCode = 403;
		std::cout << SOFT_RED "[DELETE] Cannot delete root directory (403)" << RESET << std::endl;
		return;
	}

	std::cout << SOFT_ORANGE "[DELETE] Checking if file exists..." << RESET << std::endl;

	struct stat fileInfo;
	if (stat(filePath.c_str(), &fileInfo) != 0)
	{
		this->StatusCode = 404;
		std::cout << SOFT_RED "[DELETE] File not found: " << filePath << " (404)" << RESET << std::endl;
		return;
	}
	if (S_ISDIR(fileInfo.st_mode) || !(S_ISREG(fileInfo.st_mode)))
	{
		this->StatusCode = 403; // forbidden
		std::cout << SOFT_RED "[DELETE] Cannot delete directory or irregular files: " << filePath << " (403)" << RESET << std::endl;
		return;
	}
	std::cout << SOFT_ORANGE "[DELETE] File exists and is a regular file" << RESET << std::endl;


	// check write permission
	std::string parentDir = filePath.substr(0, filePath.find_last_of('/'));
	std::cout << SOFT_ORANGE "[DELETE] Checking write permission on: " << parentDir << RESET << std::endl;
	if (access(parentDir.c_str(), W_OK) != 0)
	{
		this->StatusCode = 403;
		std::cout << SOFT_RED "[DELETE] No write permission on directory: " << parentDir << " (403)" << RESET << std::endl;
		return;
	}
	
	std::cout << SOFT_ORANGE "[DELETE] Attempting to delete file: " << filePath << RESET << std::endl;
	if (std::remove(filePath.c_str()) == 0)
	{
		this->StatusCode = 204; // no content (successful deletion)
		std::cout << SOFT_GREEN "[DELETE] Successfully deleted: " << filePath << " (204)" << RESET << std::endl;
	}
	else
	{
		this->StatusCode = 500; // internal server error
		std::cout << SOFT_RED "[DELETE] Failed to delete: " << filePath << " (500)" << RESET << std::endl;
	}
}
