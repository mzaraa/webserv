#ifndef SERVER_HPP
# define SERVER_HPP

# include "Utils.hpp"
# include "Location.hpp"

# define MAX_SIZE 90000000000000000

class Location;

class Server {
    public:
        Server();
        Server(const Server &src);
        Server &operator=(const Server &src);
        ~Server();

        /*      SETTERS       */
        void    set_port(std::string port);
        void    set_host(std::string host);
        void    set_client_max_body_size(std::string max_size);
        void    set_server_name(std::string server_name);
        void    set_error_page(std::vector<std::string> tokens);
        void    set_location(std::string path);
        void    set_socket_fd(int socket_fd);

        /*      GETTERS       */
        int                                 &get_port();
        std::string                         &get_host();
        unsigned long long                  &get_client_max_body_size();
        std::set<std::string>               &get_server_name();
        std::map<int, std::string>          &get_error_page();
        Location                            &get_location(const std::string &path);
        std::map<std::string, Location>     &get_location();
        int                                 &get_socket_fd();

        /*      DEBUG      */
        void    print_server();

    private:
        int                                         _port;
        std::string                                 _host;
        unsigned long long                          _client_max_body_size;
        std::set<std::string>                       _server_name;
        std::map<int, std::string>                  _error_page;
        std::map<std::string, Location>             _location;
        int                                         _socket_fd; // socket fd of the server
};

#endif /* SERVER_HPP */