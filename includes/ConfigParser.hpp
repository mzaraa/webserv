#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

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
# include <map>
# include <vector>
# include <set>
# include <stack>


enum    e_status {
    NONE,
    CONTINUE,
    SERVER,
    LOCATION,
    ERROR
};
# include "Utils.hpp"
# include "Server.hpp"

class Server;
class Location;

class ConfigParser {
    public:
        ConfigParser(std::string config_file, std::vector<Server> &servers);
        ConfigParser(const ConfigParser &src);
        ConfigParser &operator=(const ConfigParser &src);
        ~ConfigParser();

        /*      UTILS         */
        void    setup_server(std::vector<std::string> tokens, Server &server);
        void    setup_location(std::vector<std::string> tokens, Server &server, Location &location);
        void    parse_config_file(std::ifstream &ifs, std::vector<Server> &servers);

        /*      SETTERS       */
        void    set_server_count(int server_count);
        void    set_status(e_status status);

        /*      GETTERS       */
        int         get_server_count() const;
        e_status    get_status() const;

    private:
        int         _server_count;
        e_status    _status;
};

#endif /* CONFIGPARSER_HPP */