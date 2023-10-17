#ifndef UTILS_HPP
# define UTILS_HPP

# include "ConfigParser.hpp"
class ConfigParser;

int                         stoi( std::string &s);
void                        invalid_config_file(void);
std::string                 trim(std::string str);
std::vector<std::string>    split(std::string str, const std::string delimiter = " \t\n\r");
e_status                    line_checker(std::string line, std::stack<char> &bracket, e_status &status, ConfigParser &config);

#endif /* UTILS_HPP */