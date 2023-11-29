#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "Utils.hpp"
# include "Server.hpp"

class Socket {
    public:
        Socket(Server &server);
        Socket(const Socket &src);
        Socket &operator=(const Socket &src);
        ~Socket();

        /*      GETTERS       */
        int                 get_socket_fd() const;
        int                 get_port() const;
        std::string         get_host() const;
        struct sockaddr_in  get_addr() const;
        Server              *get_server() const;

    private:
        int                 _opt_flag;
        int                 _socket_fd;
        int                 _port;
        std::string         _host;
        struct sockaddr_in  _addr;
        Server              *_server;
    
};

#endif // SOCKET_HPP