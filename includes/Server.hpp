#ifndef SERVER_HPP
# define SERVER_HPP

# include "ConfigParser.hpp"
# include "Location.hpp"


class Server {
    public:
        Server();
        Server(const Server &src);
        Server &operator=(const Server &src);
        ~Server();

        /*      SETTERS       */
        void    set_port(std::string port);
        void    set_client_max_body_size(std::string max_size);
        void    set_server_name(std::string server_name);
        void    set_error_page(std::vector<std::string> tokens);
        void    set_location(std::string path);

        /*      GETTERS       */
        int                                 &get_port();
        int                                 &get_client_max_body_size();
        std::set<std::string>               &get_server_name();
        std::map<int, std::string>          &get_error_page();
        Location                            &get_location(const std::string &path);
        std::map<std::string, Location>     &get_location();

        /*      DEBUG      */
        void    print_server();

    private:
        int                                         _port;
        int                                         _client_max_body_size;
        std::set<std::string>                       _server_name;
        std::map<int, std::string>        _error_page;
        std::map<std::string, Location>   _location;
};

#endif /* SERVER_HPP */