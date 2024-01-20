#ifndef UTILS_HPP
# define UTILS_HPP

# include <iostream>
# include <fcntl.h>
# include <cstring>
# include <string> 
# include <unistd.h>
# include <dirent.h>
# include <sstream>
# include <cstdlib>
# include <fstream>
# include <cctype>
# include <ctime>
# include <cstdarg>
# include <sys/stat.h>
# include <sys/socket.h>
# include <sys/time.h> 
# include <sys/types.h>
# include <sys/wait.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <map>
# include <vector>
# include <set>
# include <stack>
# include <signal.h>

# include "ConfigParser.hpp"

class ConfigParser;

int                         stoi( std::string &s);
std::string                 itos(int i);
void                        invalid_config_file(void);
std::string                 trim(std::string str);
std::vector<std::string>    split(std::string str, const std::string delimiter = " \t\n\r");
e_status                    line_checker(std::string line, std::stack<char> &bracket, ConfigParser &config);
bool                        is_directory(const std::string &path);
bool                        is_file(const std::string &path);
bool                        is_ip_address(std::string str);
bool                        file_exists(const std::string &path);
bool                        exist(std::string path);
std::vector<std::string>    get_files_in_directory(const std::string &path);
void                        replace_char(std::string &str, char old_c, char new_c);
void                        to_upper(std::string &str);
std::string                 to_string(int i);

#endif /* UTILS_HPP */