#include "../includes/WebservManager.hpp"

WebservManager::WebservManager(std::vector<Server> servers) : _webserv_alive(true), _cgi_state(0), _max_fd(0), _select_ret(0){
    _vec_servers = servers;
}

WebservManager::WebservManager(const WebservManager &src) {
    *this = src;
}

WebservManager &WebservManager::operator=(const WebservManager &src) {
    if (this != &src) {
        this->_webserv_alive = src._webserv_alive;
        this->_max_fd = src._max_fd;
        this->_current_readfds = src._current_readfds;
        this->_current_writefds = src._current_writefds;
        this->_vec_servers = src._vec_servers;
        this->_clients = src._clients;
        this->_servers = src._servers;
        this->_cgi_state = src._cgi_state;
    }
    return (*this);
}

WebservManager::~WebservManager() {
}

void    WebservManager::setup_servers_socket() {
    FD_ZERO(&_current_readfds);
    FD_ZERO(&_current_writefds);
    bool    duplicate = false;
    for (std::size_t i = 0; i < _vec_servers.size(); i++) {
        for (size_t j = 0; j < i; j++) {
            if (_vec_servers[i].get_port() == _vec_servers[j].get_port() && _vec_servers[i].get_host() == _vec_servers[j].get_host()) {
                duplicate = true;
                _vec_servers[i].set_socket_fd(_vec_servers[j].get_socket_fd());
            }
        }
        if (!duplicate) {
            Socket socket(_vec_servers[i]);
            _vec_servers[i].set_socket_fd(socket.get_socket_fd());
        };
        duplicate = false;
    }
    for (std::size_t i = 0; i < _vec_servers.size(); i++) {
        if (listen(_vec_servers[i].get_socket_fd(), 10) < 0) {
            std::cerr << "Error: listen failed" << std::endl;
            exit(1);
        }
        // fcntl to make socket non-blocking
        if (fcntl(_vec_servers[i].get_socket_fd(), F_SETFL, O_NONBLOCK) < 0) {
            std::cerr << "Error: fcntl failed" << std::endl;
            exit(1);
        }
        FD_SET(_vec_servers[i].get_socket_fd(), &_current_readfds);
        _servers.insert(std::make_pair(_vec_servers[i].get_socket_fd(), _vec_servers[i]));
        _max_fd = std::max(_max_fd, _vec_servers[i].get_socket_fd());
        std::cout << "Server listening on " << _vec_servers[i].get_host() << ":" << _vec_servers[i].get_port() << std::endl;
        // //print socket_fd of _vec_servers[i] and i 
        //std::cout << "max_fd: " << _max_fd << std::endl;
    }
}

void    WebservManager::life_cycle() {
    while (_webserv_alive) {
        _readfds = _current_readfds;
        _writefds = _current_writefds;
        timeval timeout = {1, 0};
        if ((_select_ret = select(_max_fd+1, &_readfds, &_writefds, NULL, &timeout)) < 0) {
            std::cerr << "Error: select failed" << std::endl;
            exit(1);
        }
        for (int i = 0; i <= _max_fd; i++) {
            // there is a new connection request from a client
            if (FD_ISSET(i, &_readfds) && _servers.count(i) != 0) {
                std::cout << "New connection request from a client" << std::endl;
                connection_attempt(_servers[i]);
            }
            // there is a new request from a client so we need to read it
            else if (FD_ISSET(i, &_readfds) && _clients.find(i) != _clients.end()) {
                receive_request(_clients[i]);
            }
            else if (FD_ISSET(i, &_writefds) && _clients.find(i) != _clients.end()) {
                //TODO: CGI
                if ((_clients[i].get_response()->is_cgi() == false) && FD_ISSET(i, &_writefds)) {
                    std::cout << "-----------------------------------------" << std::endl;
                    std::cout << "SEND RESPONSE" << std::endl;
                    std::cout << "-----------------------------------------" << std::endl;
                    send_response(_clients[i]);
                }
            }
        }
        disconnect_client();
    }
}


void WebservManager::send_response(Client &client) {
    std::cout << "Sending response to client" << std::endl;
    std::string response = client.get_response()->get_full_response();
    int send_ret = 0;
    if (response.length() >= 42000)
        send_ret = send(client.get_fd(), response.c_str(), 42000, 0);
    else
        send_ret = send(client.get_fd(), response.c_str(), response.length(), 0);
    if (send_ret == -1) {
        close_connection(client);
    }
    else if (send_ret == 0 || (size_t)send_ret == response.length()) {
        if (client.get_response()->is_cgi() || client.get_response()->get_error_code()) {
            close_connection(client);
        }
        else {
            if (FD_ISSET(client.get_fd(), &_current_writefds)) {
                FD_CLR(client.get_fd(), &_current_writefds);
                if (client.get_fd() == _max_fd) {
                    _max_fd--;
                }
            }
            FD_SET(client.get_fd(), &_current_readfds);
            _max_fd = std::max(_max_fd, client.get_fd());
            if (client.get_request() != NULL) {
                delete client.get_request();
            }
            if (client.get_response() != NULL) {
                delete client.get_response();
            }
        }
    }
    else{
        client.get_response()->set_remaining_body(send_ret);
        client.timer();
    }
}

void    WebservManager::connection_attempt(Server &server) {
    try {
        Client client(server);
        _clients.insert(std::pair<int, Client>(client.get_fd(), client));
        FD_SET(client.get_fd(), &_current_readfds);
        _max_fd = std::max(_max_fd, client.get_fd());
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        // remove from fd_set _current_readfds and close fd
        FD_CLR(server.get_socket_fd(), &_current_readfds);
        if (server.get_socket_fd() == _max_fd) {
            _max_fd--;
        }
        close(server.get_socket_fd());
    }
}

void    WebservManager::receive_request(Client &client) {
    std::cout << "Receiving request from client" << std::endl;
    char buffer[42000];
    int read_ret = 0;
    if ((read_ret = recv(client.get_fd(), buffer, 42000, 0)) == -1) {
        std::cout << "Error: recv failed" << std::endl;
        close_connection(client);
        return ;
    }
    else if (read_ret == 0) {
        std::cout << "client disconnected" << std::endl;
        close_connection(client);
        return ;
    }
    else if (read_ret > 0) {
        if (!client.has_received_header()){
            //std::cout << "Receiving header from client" << std::endl;
            client.get_raw_request().append(buffer, read_ret);
            //std::cout << "Raw request: " << client.get_raw_request() << std::endl;
            if (client.get_raw_request().find("\r\n\r\n")) {
                //std::cout << "Header received" << std::endl;
                client.get_request()->parse_request(client.get_raw_request());
            }
            client.timer();
        }
        else if (!client.get_request()->full_body_received()) {
            // append to body
            client.get_raw_request().append(buffer, read_ret);
            client.get_request()->set_remaining_body(client.get_request()->get_remaining_body() - read_ret);
            if (client.get_request()->get_remaining_body() <= 0) {
                client.get_request()->set_full_body_received(true);
            }
        }
    }
    if (client.get_request()->full_body_received()) {
        // remove from fd_set _current_readfds and add to _current_writefds
        if (FD_ISSET(client.get_fd(), &_current_readfds)) {
            FD_CLR(client.get_fd(), &_current_readfds);
            if (client.get_fd() == _max_fd) {
                _max_fd--;
            }
        }
        FD_SET(client.get_fd(), &_current_writefds);
        _max_fd = std::max(_max_fd, client.get_fd());
        // match server
        match_server(client);
        // TODO: prepare response
        client.get_response()->set_server(client.get_server());
        client.get_response()->set_request(client.get_request());
        client.get_response()->create_response();
        // client.get_request()->print_request();
        std::cout << "TOUT est OK" << std::endl;
        if (client.get_response()->is_cgi()) {
            // add to _current_writefds pipe_fd_in[1]
            FD_SET(client.get_response()->cgi->_pipe_fd_in[1], &_current_writefds);
            _max_fd = std::max(_max_fd, client.get_response()->cgi->_pipe_fd_in[1]);
            // add to _current_readfds pipe_fd_out[0]
            FD_SET(client.get_response()->cgi->_pipe_fd_out[0], &_current_readfds);
            _max_fd = std::max(_max_fd, client.get_response()->cgi->_pipe_fd_out[0]);
            std::cout << "CGI" << std::endl;
        }
    }
}

void    WebservManager::close_connection(Client &client) {
    // remove from fd_set _current_writefds and close fd
    if (FD_ISSET(client.get_fd(), &_current_writefds)) {
        FD_CLR(client.get_fd(), &_current_writefds);
        if (client.get_fd() == _max_fd) {
            _max_fd--;
        }
    }
    if (FD_ISSET(client.get_fd(), &_current_readfds)) {
        FD_CLR(client.get_fd(), &_current_readfds);
        if (client.get_fd() == _max_fd) {
            _max_fd--;
        }
    }
    close(client.get_fd());
    // clear allocated memory
    delete client.get_request();
    delete client.get_response()->cgi;
    delete client.get_response();
    // remove from _clients
    _clients.erase(client.get_fd());
    // print in red and bold a client has been disconnected
    std::cout << "\033[1;31mClient disconnected\033[0m" << std::endl;
}

void    WebservManager::match_server(Client &client) {
    for (std::map<int, Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
        if (it->second.get_port() != client.get_request()->get_port())
            continue ;
        if (!client.get_request()->get_server_name().empty() && it->second.get_server_name().find(client.get_request()->get_server_name()) != it->second.get_server_name().end()) {
            client.set_server(it->second);
            return ;
        }
        if (it->second.get_host() != client.get_request()->get_host()){
            continue ;
        }
        client.set_server(it->second);
        return ;
    }
}

void   WebservManager::disconnect_client() {
    time_t current_time = time(NULL);
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++) {
        if ((current_time - it->second.get_last_activity()) > 60) {
            close_connection(it->second);
        }
    }
}