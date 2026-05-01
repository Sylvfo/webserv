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