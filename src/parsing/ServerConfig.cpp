/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: beboccas <beboccas@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 15:07:41 by beboccas          #+#    #+#             */
/*   Updated: 2025/11/14 03:03:50 by beboccas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>

ServerConfig WebServ::getServer(int index)
{
	if (index < 0 || static_cast<size_t>(index) >= servers.size())
		throw std::out_of_range("Server index out of range");
	return servers[index];
}

std::string toString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

//hostReq format: "hostname:port"
ServerConfig WebServ::getServer(std::string hostReq)
{
	ServerConfig emptyServer;
	emptyServer.listen_port = -1;
	emptyServer.server_name = "";
	//for each server create string that match the request format
	std::string serverHostPort;
	for (size_t i = 0; i < servers.size(); ++i)
	{
		serverHostPort = servers[i].server_name + ":" + toString(servers[i].listen_port);
		if (serverHostPort == hostReq)
			return servers[i];
	}
	std::cout << "No server matches the requested host:" << hostReq << ". Returning the first one." << std::endl;
	return servers[0];//return first server as default
	
}

void WebServ::addServer(ServerConfig config)
{
	servers.push_back(config);
}

std::vector<ServerConfig> WebServ::getServerList()
{
	return servers;
}

void WebServ::printConfig()
{
	for (size_t i = 0; i < servers.size(); ++i)
	{
		const ServerConfig &s = servers[i];
		std::cout << "Server[" << i << "] listen=" << s.listen_port << " name=" << s.server_name << " root=" << s.root << " body_max=" << s.client_max_body_size << std::endl;
		for (std::map<int, std::string>::const_iterator it = s.error_pages.begin(); it != s.error_pages.end(); ++it)
			std::cout << "  error " << it->first << " -> " << it->second << std::endl;
		for (size_t j = 0; j < s.locations.size(); ++j)
		{
			const LocationConfig &l = s.locations[j];
			std::cout << "  location " << l.path << " root=" << l.root << " index=" << l.index << " autoindex=" << (l.autoindex?"on":"off") << std::endl;
			std::cout << "    methods:";
			for (size_t k = 0; k < l.methods.size(); ++k) std::cout << " " << l.methods[k];
			std::cout << std::endl;
			for (size_t k = 0; k < l.returns.size(); ++k)
				std::cout << "    return " << l.returns[k].first << " " << l.returns[k].second << std::endl;
		}
	}
}

void WebServ::parseConfig(std::string path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		throw std::runtime_error("Cannot open config file: " + path);

	std::string line;
	ServerConfig currentServer;
	LocationConfig currentLocation;
	bool inServer = false;
	bool inLocation = false;

	int lineNumber = 0;

	while (std::getline(file, line))
	{
		lineNumber++;
		std::istringstream iss(line);
		std::string key;
		if (!(iss >> key))
			continue; // ligne vide ou espaces

		if (key[0] == '#')
			continue; // commentaire

		// ----- Début de bloc serveur -----
		if (key == "server")
		{
			std::string brace;
			iss >> brace;
			if (brace != "{")
			{
				std::ostringstream oss;
				oss << "Expected '{' after server (line " << lineNumber << ")";
				throw std::runtime_error(oss.str());
			}
			if (inServer)
			{
				std::ostringstream oss;
				oss << "Nested server block not allowed (line " << lineNumber << ")";
				throw std::runtime_error(oss.str());
			}
			inServer = true;
			currentServer = ServerConfig();
			continue;
		}

		// ----- Fermeture de bloc -----
		if (key == "}")
		{
			if (inLocation)
			{
				currentServer.locations.push_back(currentLocation);
				inLocation = false;
			}
			else if (inServer)
			{
				this->addServer(currentServer);
				inServer = false;
			}
			else
			{
				std::ostringstream oss;
				oss << "Unexpected '}' (line " << lineNumber << ")";
				throw std::runtime_error(oss.str());
			}
			continue;
		}

		// ----- Directives serveur -----
		if (inServer && !inLocation)
		{
			if (key == "listen")
			{
				iss >> currentServer.listen_port;
			}
			else if (key == "server_name")
			{
				std::string tmp;
				iss >> tmp;
				if (!tmp.empty() && tmp[tmp.size() - 1] == ';')
					tmp = tmp.substr(0, tmp.size() - 1);
				currentServer.server_name = tmp;
			}
			else if (key == "root")
			{
				std::string tmp;
				iss >> tmp;
				if (!tmp.empty() && (tmp[tmp.size() - 1] == ';' || tmp[tmp.size() - 1] == '{'))
					tmp = tmp.substr(0, tmp.size() - 1);
				currentServer.root = tmp;
			}
			else if (key == "client_max_body_size")
			{
				iss >> currentServer.client_max_body_size;
			}
			else if (key == "error_page")
			{
				// read remaining tokens: many status codes followed by a path
				std::vector<std::string> tokens;
				std::string tok;
				while (iss >> tok)
				{
					if (!tok.empty() && tok[tok.size() - 1] == ';')
						tok = tok.substr(0, tok.size() - 1);
					tokens.push_back(tok);
				}
				if (tokens.size() >= 2)
				{
					std::string pathErr = tokens.back();
					for (size_t i = 0; i + 1 < tokens.size(); ++i)
					{
						int code = std::atoi(tokens[i].c_str());
						currentServer.error_pages[code] = pathErr;
					}
				}
			}
			else if (key == "location")
			{
				if (inLocation)
				{
					std::ostringstream oss;
					oss << "Nested location not allowed (line " << lineNumber << ")";
					throw std::runtime_error(oss.str());
				}
				inLocation = true;
				currentLocation = LocationConfig();
				iss >> currentLocation.path;

				std::string brace;
				iss >> brace;
				if (brace != "{")
				{
					std::ostringstream oss;
					oss << "Expected '{' after location (line " << lineNumber << ")";
					throw std::runtime_error(oss.str());
				}
			}
			else
			{
				std::cerr << "[WARN] Unknown directive in server: " << key << " (line " << lineNumber << ")\n";
			}
			continue;
		}

		// ----- Directives location -----
		if (inLocation)
		{
			if (key == "methods" || key == "method")
			{
				std::string method;
				while (iss >> method)
				{
					if (!method.empty() && method[method.size() - 1] == ';')
						method = method.substr(0, method.size() - 1);
					currentLocation.methods.push_back(method);
				}
			}
			else if (key == "autoindex")
			{
				std::string val;
				iss >> val;
				currentLocation.autoindex = (val == "on");
			}
			else if (key == "index")
			{
				std::string tmp;
				iss >> tmp;
				if (!tmp.empty() && tmp[tmp.size() - 1] == ';')
					tmp = tmp.substr(0, tmp.size() - 1);
				currentLocation.index = tmp;
			}
			else if (key == "root")
			{
				std::string tmp;
				iss >> tmp;
				if (!tmp.empty() && tmp[tmp.size() - 1] == ';')
					tmp = tmp.substr(0, tmp.size() - 1);
				currentLocation.root = tmp;
			}
			else if (key == "upload_path")
				iss >> currentLocation.upload_path;
			else if (key == "cgi_path")
				iss >> currentLocation.cgi_path;
			else if (key == "return")
			{
				int code;
				std::string pathReturn;
				iss >> code >> pathReturn;
				if (!pathReturn.empty() && pathReturn[pathReturn.size() - 1] == ';')
					pathReturn = pathReturn.substr(0, pathReturn.size() - 1);
				currentLocation.returns.push_back(std::make_pair(code, pathReturn));
			}
			else
				std::cerr << "[WARN] Unknown directive in location: " << key << " (line " << lineNumber << ")\n";
			continue;
		}

		std::cerr << "[WARN] Unknown directive outside server block: " << key << " (line " << lineNumber << ")\n";
	}

	// ----- Vérifications de fin -----
	if (inLocation)
	{
		std::ostringstream oss;
		oss << "Unclosed location block at EOF (line " << lineNumber << ")";
		throw std::runtime_error(oss.str());
	}
	if (inServer)
	{
		this->addServer(currentServer);
	}
}