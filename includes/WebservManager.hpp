#ifndef WEBSERVMANAGER_HPP
# define WEBSERVMANAGER_HPP

# include "Utils.hpp"
# include "Server.hpp"
# include "Client.hpp"
# include "Socket.hpp"

class Server;
class Client;
class Socket;

class WebservManager {
    public:
        WebservManager(std::vector<Server> servers);
        WebservManager(const WebservManager &src);
        WebservManager &operator=(const WebservManager &src);
        ~WebservManager();

        void    setup_servers_socket();
        void    life_cycle();
        void    connection_attempt(Server &server);
        void    receive_request(Client &client);
        void    send_response(Client &client);
        void    close_connection(Client &client);
        void    match_server(Client &client);
        void    disconnect_client();

    private:
        bool _webserv_alive;
        int _max_fd;
        int _select_ret;
        fd_set _current_readfds;
        fd_set _current_writefds;
        fd_set _readfds;
        fd_set _writefds;
        std::vector<Server> _vec_servers;
        std::map<int, Client> _clients;
        std::map<int, Server> _servers;
};

#endif // WEBSERVMANAGER_HPP