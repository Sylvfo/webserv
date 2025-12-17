#include "Webserv.hpp"
#include <dirent.h>
#include <algorithm>

std::string HttpRequest::generate_directory_listing(const std::string &dir_path, const std::string &uri_path)
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

void HttpRequest::GetRequest()
{
	if (GetAccessRessource() == true)
	{
		if (AnswerBody.empty())
		{
			if (loadRessource() == true)
			{
				StatusCode = 200;
			}
		}
	}
	else
	{
		AnswerType = ERROR;
	}
}

bool HttpRequest::GetAccessRessource()
{
	std::string makingPath = this->Path;
	std::string decodedUri = urlDecode(this->uri);
	
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
			StatusCode = 301; // Moved Permanently
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
				std::string listing = generate_directory_listing(makingPath, decodedUri);
				if (!listing.empty())
				{
					AnswerBody = listing;
					ContentLength = AnswerBody.size();
					ContentType = "text/html";
					StatusCode = 200;
					fd_Ressource = -1; // No file descriptor needed
					return true;
				}
			}
			
			StatusCode = 403;
			AnswerType = ERROR;
			return false;
		}
		
		// Index file exists, use it
		makingPath = indexPath;
	}
	const char *path = makingPath.c_str();
	
	// Check if file exists first
	if (access(path, F_OK) != 0)
	{
		StatusCode = 404;
		AnswerType = ERROR;
		return (false);
	}
	
	// Check if we have read permission
	if (access(path, R_OK) != 0)
	{
		StatusCode = 403;
		AnswerType = ERROR;
		return (false);
	}
	
	fd_Ressource = open(path , O_RDONLY);
	if (fd_Ressource < 0)
	{
		StatusCode = 500;
		AnswerType = ERROR;
		return (false);
	}
	SetContentType(makingPath);
	return true;
}

bool HttpRequest::loadRessource()
{
	AnswerBody.clear();
	char buff[4096];
	ssize_t bytesRead;

	if (fd_Ressource == -1)
	{
		UseDefaultErrorHTML();
		return true;
	}
	
	try {
		// Get file size and reserve memory to avoid reallocations
		off_t fileSize = lseek(fd_Ressource, 0, SEEK_END);
		lseek(fd_Ressource, 0, SEEK_SET); // Reset to beginning
		
		if (fileSize > 0)
		{
			AnswerBody.reserve(fileSize);
		}
		
		while ((bytesRead = read(fd_Ressource, buff, sizeof(buff))) > 0)
		{
			AnswerBody.append(buff, bytesRead);
		}
		//check eof
		close(fd_Ressource);
		ContentLength = AnswerBody.size();
		return true;
		
	} catch (const std::bad_alloc& e) {
		close(fd_Ressource);
		StatusCode = 500;
		AnswerBody = "Internal Server Error: File too large";
		ContentLength = AnswerBody.size();
		return false;
	} catch (const std::exception& e) {
		close(fd_Ressource);
		StatusCode = 500;
		return false;
	}
}

void HttpRequest::SetContentType(std::string &makingPath)
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
		ContentType = "application/octet-stream";
	}
	else
	{
		try {
			ContentType = it->second;
		} catch (const std::bad_alloc& e) {
			ContentType = "application/octet-stream";
		}
	}
}

