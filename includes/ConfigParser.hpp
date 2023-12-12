#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP


enum    e_status {
    NONE,
    CONTINUE,
    SERVER,
    LOCATION,
    ERROR
};

# include "Server.hpp"
# include "Utils.hpp"
# include "WebservManager.hpp"

// class Server;
class Location;

class ConfigParser {
    public:
        ConfigParser(std::string config_file, std::vector<Server> &servers);
        ConfigParser(const ConfigParser &src);
        ConfigParser &operator=(const ConfigParser &src);
        ~ConfigParser();

        /*      UTILS         */
        void    setup_server(std::vector<std::string> tokens, Server &server);
        void    setup_location(std::vector<std::string> tokens, Location &location);
        void    setup_location_cgi(std::vector<std::string> tokens, Location &location);
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