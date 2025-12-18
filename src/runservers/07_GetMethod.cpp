#include "Webserv.hpp"
#include <dirent.h>
#include <algorithm>

std::string HttpRequest::_generate_directory_listing(const std::string &dir_path, const std::string &uri_path)
{
	DIR *dir = opendir(dir_path.c_str());
	if (!dir)
	{
		return "";
	}

	std::vector<std::string> entries;
	struct dirent *entry;
	
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		
		if (name == "." || name == "..")
			continue;
			
		entries.push_back(name);
	}
	closedir(dir);
	std::sort(entries.begin(), entries.end());

	std::string html;
	html = "<!DOCTYPE html>\n";
	html += "<html>\n<head>\n";
	html += "<meta charset=\"UTF-8\">\n";
	html += "<title>Index of " + uri_path + "</title>\n";
	html += "<style>\n";
	html += "body { font-family: monospace; margin: 40px; }\n";
	html += "h1 { border-bottom: 1px solid #ccc; padding-bottom: 10px; }\n";
	html += "ul { list-style: none; padding: 0; }\n";
	html += "li { padding: 5px 0; }\n";
	html += "a { text-decoration: none; color: #0066cc; }\n";
	html += "a:hover { text-decoration: underline; }\n";
	html += ".dir::before { content: '📁 '; }\n";
	html += ".file::before { content: '📄 '; }\n";
	html += "</style>\n";
	html += "</head>\n<body>\n";
	html += "<h1>Index of " + uri_path + "</h1>\n";
	html += "<ul>\n";
	
	if (uri_path != "/")
	{
		html += "<li><a href=\"../\" class=\"dir\">../</a></li>\n";
	}
	for (size_t i = 0; i < entries.size(); ++i)
	{
		std::string name = entries[i];
		std::string full_path = dir_path;
		if (full_path[full_path.length() - 1] != '/')
			full_path += "/";
		full_path += name;
		
		struct stat entry_stat;
		std::string css_class = "file";
		std::string link_suffix = "";
		
		if (stat(full_path.c_str(), &entry_stat) == 0)
		{
			if (S_ISDIR(entry_stat.st_mode))
			{
				css_class = "dir";
				link_suffix = "/";
			}
		}
		
		html += "<li><a href=\"" + name + link_suffix + "\" class=\"" + css_class + "\">" + name + link_suffix + "</a></li>\n";
	}
	html += "</ul>\n";
	html += "</body>\n</html>\n";
	return html;
}

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
	
	// Check if path is a directory
	struct stat pathStat;
	if (stat(makingPath.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode))
	{
		// If URI doesn't end with '/', redirect to add it
		if (!decodedUri.empty() && decodedUri[decodedUri.length() - 1] != '/')
		{
			status_code = 301; // Moved Permanently
			// The redirect will be handled by the response builder
			return false;
		}
		
		std::string index = matchingLocation ? matchingLocation->index : "index.html";
		if (index.empty())
			index = "index.html";
		
		// Append index file to directory path
		std::string indexPath = makingPath;
		if (indexPath[indexPath.length() - 1] != '/')
			indexPath += "/";
		indexPath += index;
		
		// Check if index file exists in directory
		if (access(indexPath.c_str(), F_OK) != 0)
		{
			// Index file doesn't exist, check if autoindex is enabled
			if (matchingLocation && matchingLocation->autoindex)
			{
				// Generate directory listing
				std::string listing = _generate_directory_listing(makingPath, decodedUri);
				if (!listing.empty())
				{
					answer_body = listing;
					content_length = answer_body.size();
					content_type = "text/html";
					status_code = 200;
					fd_ressource = -1; // No file descriptor needed
					return true;
				}
			}
			
			status_code = 403;
			answer_type = ERROR;
			return false;
		}
		
		// Index file exists, use it
		makingPath = indexPath;
	}
	const char *path = makingPath.c_str();
	
	// Check if file exists first
	if (access(path, F_OK) != 0)
	{
		status_code = 404;
		answer_type = ERROR;
		return (false);
	}
	
	// Check if we have read permission
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
		// Get file size and reserve memory to avoid reallocations
		off_t fileSize = lseek(fd_ressource, 0, SEEK_END);
		lseek(fd_ressource, 0, SEEK_SET); // Reset to beginning
		
		if (fileSize > 0)
		{
			answer_body.reserve(fileSize);
		}
		
		while ((bytesRead = read(fd_ressource, buff, sizeof(buff))) > 0)
		{
			answer_body.append(buff, bytesRead);
		}
		//check eof
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

