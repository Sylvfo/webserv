#include "Webserv.hpp"
#include <dirent.h>
#include <algorithm>

void HttpRequest::getRequest()
{
	if (_getAccessRessource() == true)
	{
		if (answer_body.empty())
		{
			if (_loadRessource() == true)
			{
				status_code = 200;
			}
		}
	}
	else
	{
		answer_type = ERROR;
	}
}

bool HttpRequest::_getAccessRessource()
{
	std::string makingPath = this->path;
	std::string decodedUri = _urlDecode(this->uri);
	
	LocationConfig* matchingLocation = NULL;
	size_t bestMatchLength = 0;

	for (size_t i = 0; i < Server->locations.size(); ++i)
	{
		const std::string& locationPath = Server->locations[i].path;
		if (decodedUri.find(locationPath) == 0 && locationPath.length() > bestMatchLength)
		{
			matchingLocation = &Server->locations[i];
			bestMatchLength = locationPath.length();
		}
	}
		struct stat pathStat;
	if (stat(makingPath.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode))
	{
		if (!decodedUri.empty() && decodedUri[decodedUri.length() - 1] != '/')
		{
			status_code = 301;
			return false;
		}

		std::string index = matchingLocation ? matchingLocation->index : "index.html";
		if (index.empty())
			index = "index.html";
		
		std::string indexPath = makingPath;
		if (indexPath[indexPath.length() - 1] != '/')
			indexPath += "/";
		indexPath += index;
		
		if (access(indexPath.c_str(), F_OK) != 0)
		{
			if (matchingLocation && matchingLocation->autoindex)
			{
				std::string listing = _generate_directory_listing(makingPath, decodedUri);
				if (!listing.empty())
				{
					answer_body = listing;
					content_length = answer_body.size();
					content_type = "text/html";
					status_code = 200;
					fd_ressource = -1;
					return true;
				}
			}
			status_code = 403;
			answer_type = ERROR;
			return false;
		}
		
		makingPath = indexPath;
	}
	const char *path = makingPath.c_str();
	
	if (access(path, F_OK) != 0)
	{
		status_code = 404;
		answer_type = ERROR;
		return (false);
	}
	if (access(path, R_OK) != 0)
	{
		status_code = 403;
		answer_type = ERROR;
		return (false);
	}
	fd_ressource = open(path , O_RDONLY);
	if (fd_ressource < 0)
	{
		status_code = 500;
		answer_type = ERROR;
		return (false);
	}
	_setcontent_type(makingPath);
	return true;
}

bool HttpRequest::_loadRessource()
{
	answer_body.clear();
	char buff[4096];
	ssize_t bytesRead;

	if (fd_ressource == -1)
	{
		useDefaultErrorHTML();
		return true;
	}

	try {
		off_t fileSize = lseek(fd_ressource, 0, SEEK_END);
		lseek(fd_ressource, 0, SEEK_SET);
		
		if (fileSize > 0)
		{
			answer_body.reserve(fileSize);
		}
		
		while ((bytesRead = read(fd_ressource, buff, sizeof(buff))) > 0)
		{
			answer_body.append(buff, bytesRead);
		}
		close(fd_ressource);
		content_length = answer_body.size();
		return true;

	} catch (const std::bad_alloc& e) {
		close(fd_ressource);
		status_code = 500;
		answer_body = "Internal Server Error: File too large";
		content_length = answer_body.size();
		return false;
	} catch (const std::exception& e) {
		close(fd_ressource);
		status_code = 500;
		return false;
	}
}

void HttpRequest::_setcontent_type(std::string &makingPath)
{
	std::string Extension;
	size_t dot = makingPath.find_last_of('.');
	if (dot == std::string::npos)
	{
		Extension = "defaut";
	}
	else
	{
		Extension = makingPath.substr(dot);
	}
	std::map<std::string, std::string>::iterator it = Server->mime_types.find(Extension);
	if (it == Server->mime_types.end())
	{
		content_type = "application/octet-stream";
	}
	else
	{
		try {
			content_type = it->second;
		} catch (const std::bad_alloc& e) {
			content_type = "application/octet-stream";
		}
	}
}
