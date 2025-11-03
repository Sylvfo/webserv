#ifndef PARSING_HPP
#define PARSING_HPP

#include "webserv.hpp"
#include "DataStructureWebServ.hpp"

bool checkArgs(int argc, char **argv);


//ServerConfig
Config parse_config(const std::string &path);
void print_config(const Config &cfg);

#endif