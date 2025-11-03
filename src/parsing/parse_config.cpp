/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_config.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: beboccas <beboccas@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/17 13:50:08 by beboccas          #+#    #+#             */
/*   Updated: 2025/11/03 15:51:56 by beboccas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <iostream>
#include <cstdlib>

#include "Webserv.hpp"

static std::string trim(const std::string &s)
{
	size_t i = 0;
	size_t j = s.size();
	while (i < s.size() && isspace(static_cast<unsigned char>(s[i]))) ++i;
	while (j > i && isspace(static_cast<unsigned char>(s[j - 1]))) --j;
	return s.substr(i, j - i);
}

static std::vector<std::string> split_ws(const std::string &line)
{
	std::vector<std::string> out;
	std::istringstream iss(line);
	std::string tok;
	while (iss >> tok)
		out.push_back(tok);
	return out;
}
static void strip_trailing_semicolon(std::vector<std::string> &toks)
{
	if (toks.empty()) return;
	std::string &last = toks.back();
	if (!last.empty() && last[last.size() - 1] == ';')
		last = last.substr(0, last.size() - 1);
}

static void start_server(Config &cfg, ServerConfig *&current_server, LocationConfig *&current_location)
{
	cfg.servers.push_back(ServerConfig());
	current_server = &cfg.servers.back();
	current_location = NULL;
}

static void start_location(ServerConfig *current_server, LocationConfig *&current_location, const std::string &header, int lineno)
{
	if (!current_server)
	{
		std::ostringstream oss;
		oss << "location outside server at line " << lineno;
		throw std::runtime_error(oss.str());
	}
	std::string line = header;
	size_t pos = line.find('{');
	if (pos != std::string::npos) line = line.substr(0, pos);
	std::vector<std::string> toks = split_ws(line);
	if (toks.size() < 2)
	{
		std::ostringstream oss;
		oss << "Invalid location header at line " << lineno;
		throw std::runtime_error(oss.str());
	}
	current_server->locations.push_back(LocationConfig());
	current_location = &current_server->locations.back();
	current_location->path = toks[1];
}

static void handle_directive(const std::vector<std::string> &toks, ServerConfig *current_server, LocationConfig *current_location, int lineno)
{
	if (toks.empty()) return;
	const std::string &key = toks[0];

	if (key == "listen")
	{
		if (!current_server)
		{
			std::ostringstream oss; oss << "listen outside server at line " << lineno; throw std::runtime_error(oss.str());
		}
		if (toks.size() < 2) return;
		current_server->listen_port = std::atoi(toks[1].c_str());
	}
	else if (key == "server_name")
	{
		if (!current_server) throw std::runtime_error("server_name outside server");
		if (toks.size() < 2) return;
		current_server->server_name = toks[1];
	}
	else if (key == "error_page")
	{
		if (!current_server) throw std::runtime_error("error_page outside server");
		if (toks.size() < 3) return;
		std::string path = toks.back();
		for (size_t i = 1; i + 1 < toks.size(); ++i)
		{
			int code = std::atoi(toks[i].c_str());
			current_server->error_pages[code] = path;
		}
	}
	else if (key == "root")
	{
		std::string value;
		if (toks.size() >= 2) value = toks[1];
		if (current_location) current_location->root = value;
		else if (current_server) current_server->root = value;
	}
	else if (key == "client_max_body_size")
	{
		if (!current_server) throw std::runtime_error("client_max_body_size outside server");
		if (toks.size() >= 2) current_server->client_max_body_size = std::atol(toks[1].c_str());
	}
	else if (key == "method" || key == "methods")
	{
		if (!current_location) throw std::runtime_error("method outside location");
		current_location->methods.clear();
		for (size_t i = 1; i < toks.size(); ++i) current_location->methods.push_back(toks[i]);
	}
	else if (key == "autoindex")
	{
		if (!current_location) throw std::runtime_error("autoindex outside location");
		if (toks.size() >= 2 && toks[1] == "on") current_location->autoindex = true;
		else current_location->autoindex = false;
	}
	else if (key == "index")
	{
		if (!current_location) throw std::runtime_error("index outside location");
		if (toks.size() >= 2) current_location->index = toks[1];
	}
	else if (key == "upload_path")
	{
		if (!current_location) throw std::runtime_error("upload_path outside location");
		if (toks.size() >= 2) current_location->upload_path = toks[1];
	}
	else if (key == "cgi_path")
	{
		if (!current_location) throw std::runtime_error("cgi_path outside location");
		if (toks.size() >= 2) current_location->cgi_path = toks[1];
	}
	else if (key == "return")
	{
		if (!current_location) throw std::runtime_error("return outside location");
		if (toks.size() >= 3)
		{
			int code = std::atoi(toks[1].c_str());
			std::string url = toks[2];
			current_location->returns.push_back(std::make_pair(code, url));
		}
	}
	else
	{
		// unknown directive
	}
}

Config parse_config(const std::string &path)
{
	std::ifstream ifs(path.c_str());
	if (!ifs) throw std::runtime_error(std::string("Unable to open config file: ") + path);

	Config cfg;
	std::string line;
	ServerConfig *current_server = NULL;
	LocationConfig *current_location = NULL;
	int lineno = 0;

	while (std::getline(ifs, line))
	{
		++lineno;
		std::string s = trim(line);
		if (s.empty()) continue;
		if (s[0] == '#') continue;

		if (s == "server {")
		{
			start_server(cfg, current_server, current_location);
			continue;
		}

		if (s == "}")
		{
			if (current_location) current_location = NULL;
			else current_server = NULL;
			continue;
		}

		if (s.size() >= 9 && s.substr(0, 9) == "location ")
		{
			start_location(current_server, current_location, s, lineno);
			continue;
		}

		std::vector<std::string> toks = split_ws(s);
		strip_trailing_semicolon(toks);
		if (toks.empty()) continue;

		handle_directive(toks, current_server, current_location, lineno);
	}

	return cfg;
}

void print_config(const Config &cfg)
{
	for (size_t i = 0; i < cfg.servers.size(); ++i)
	{
		const ServerConfig &s = cfg.servers[i];
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

