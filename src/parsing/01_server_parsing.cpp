/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: beboccas <beboccas@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/05 15:07:41 by beboccas          #+#    #+#             */
/*   Updated: 2025/12/18 15:05:42 by beboccas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>


void WebServ::addServer(ServerConfig config)
{
	servers.push_back(config);
}

std::vector<ServerConfig> WebServ::getServerList()
{
	return servers;
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
				if (!(iss >> currentServer.listen_port))
				{
					std::ostringstream oss;
					oss << "Invalid port number (line " << lineNumber << ")";
					throw std::runtime_error(oss.str());
				}
				if (currentServer.listen_port < 1 || currentServer.listen_port > 65535)
				{
					std::ostringstream oss;
					oss << "Port number out of range (1-65535): " << currentServer.listen_port << " (line " << lineNumber << ")";
					throw std::runtime_error(oss.str());
				}
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
						std::stringstream sToken(tokens[i]);
						int code;
						sToken >> code;
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
			if (key == "location")
			{
				std::ostringstream oss;
				oss << "Nested location blocks not allowed (line " << lineNumber << ")";
				throw std::runtime_error(oss.str());
			}
			else if (key == "methods" || key == "method")
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
				if (!val.empty() && val[val.size() - 1] == ';')
					val = val.substr(0, val.size() - 1);
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
			{
				std::string tmp;
				iss >> tmp;
				if (!tmp.empty() && tmp[tmp.size() - 1] == ';')
					tmp = tmp.substr(0, tmp.size() - 1);
				currentLocation.upload_path = tmp;
			}
			else if (key == "cgi_path")
			{
				std::string tmp;
				iss >> tmp;
				if (!tmp.empty() && tmp[tmp.size() - 1] == ';')
					tmp = tmp.substr(0, tmp.size() - 1);
				currentLocation.cgi_path = tmp;
			}
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
		std::ostringstream oss;
		oss << "Unclosed server block at EOF (line " << lineNumber << ")";
		throw std::runtime_error(oss.str());
	}

	// Vérifier qu'au moins un serveur a été configuré
	if (servers.empty())
	{
		throw std::runtime_error("No server blocks found in config file");
	}

	// Valider chaque serveur
	for (size_t i = 0; i < servers.size(); ++i)
	{
		const ServerConfig &s = servers[i];

		// Vérifier que le port est défini
		if (s.listen_port <= 0)
		{
			std::ostringstream oss;
			oss << "Server[" << i << "] missing or invalid listen port";
			throw std::runtime_error(oss.str());
		}

		// Vérifier que root est défini
		if (s.root.empty())
		{
			std::ostringstream oss;
			oss << "Server[" << i << "] (port " << s.listen_port << ") missing root directive";
			throw std::runtime_error(oss.str());
		}
	}
}
