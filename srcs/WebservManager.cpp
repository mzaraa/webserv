#include "../includes/WebservManager.hpp"

WebservManager::WebservManager(std::vector<Server> servers) : _webserv_alive(true), _max_fd(0), _select_ret(0){
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
                // green and bold print new connection request from a client
                std::cout << "\033[1;32m";
                std::cout << "New connection request from a client" << std::endl;
                std::cout << "\033[0m";
                connection_attempt(_servers[i]);
            }
            // there is a new request from a client so we need to read it
            else if (FD_ISSET(i, &_readfds) && _clients.find(i) != _clients.end()) {
                receive_request(_clients[i]);
            }
            else if (FD_ISSET(i, &_writefds) && _clients.find(i) != _clients.end()) {
                if (_clients[i].get_response()->_cgi_state == 1 && FD_ISSET(_clients[i].get_response()->cgi->_pipe_fd_in[1], &_writefds)) {
                    // std::cout << "-----------------------------------------" << std::endl;
                    // std::cout << "SEND CGI" << std::endl;
                    // std::cout << "-----------------------------------------" << std::endl;
                    send_cgi(_clients[i]);
                }
                else if (_clients[i].get_response()->_cgi_state == 1 && FD_ISSET(_clients[i].get_response()->cgi->_pipe_fd_out[0], &_readfds)) {
                    // std::cout << "-----------------------------------------" << std::endl;
                    // std::cout << "READ CGI" << std::endl;
                    // std::cout << "-----------------------------------------" << std::endl;
                    read_cgi(_clients[i]);
                }
                else if (( _clients[i].get_response()->_cgi_state == 2 || _clients[i].get_response()->_cgi_state == 0) && FD_ISSET(i, &_writefds)) {
                    // std::cout << "-----------------------------------------" << std::endl;
                    // std::cout << "SEND RESPONSE" << std::endl;
                    // std::cout << "-----------------------------------------" << std::endl;
                    send_response(_clients[i]);
                }
            }
        }
        disconnect_client();
    }
}


void WebservManager::send_response(Client &client) {
    // std::cout << "Sending response to client" << std::endl;
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
        // print the error code sent to the client in blue but not bold
        if (client.get_response()->get_error_code()) {
            std::cout << "\033[0;34m";
            std::cout << "Status code sent to the client: " << client.get_response()->get_error_code() << std::endl;
            std::cout << "\033[0m";
        }
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
    // std::cout << "Receiving request from client" << std::endl;
    char buffer[42000];
    int read_ret = 0;
    if ((read_ret = read(client.get_fd(), buffer, 42000)) == -1){
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
        // std::cout << "-----------------------------------------" << std::endl;
        std::cout << "Read " << read_ret << " bytes from client" << std::endl;
        if (!client.has_received_header()){
            client.get_raw_request().append(buffer, read_ret);
            //std::cout << "Raw request: " << client.get_raw_request() << std::endl;
            if (client.get_raw_request().find("\r\n\r\n")) {
                //std::cout << "Header received" << std::endl;
                client.get_request()->parse_request(client.get_raw_request());
                //set _has_received_header to true
                client.set_has_received_header(true);
            }
            // calcule the remaining body to receive because we received a part of the body with the header
            // client.get_request()->set_remaining_body(client.get_request()->get_content_length() - (client.get_raw_request().length() - client.get_raw_request().find("\r\n\r\n") - 4));
            client.timer();
        }
        else if (client.get_request()->full_body_received() == false && client.has_received_header() == true){
            // std::cout << "Receiving body from client" << std::endl;
            client.get_request()->get_body().append(buffer, read_ret);
            client.get_request()->set_remaining_body(client.get_request()->get_remaining_body() - read_ret);
            std::cout << "Remaining body after HEADER: " << client.get_request()->get_remaining_body() << std::endl;
            // print remaining body to receive
            if (client.get_request()->get_remaining_body() <= 0) {
                client.get_request()->set_full_body_received(true);
            }
            //print value of _remaining_body
            // std::cout << "Remaining body: " << client.get_request()->get_remaining_body() << std::endl;
            // client.timer();
        }
    }
    if (client.get_request()->full_body_received() && client.has_received_header()) {
        // std::cout << "FULL BODY RECEIVED" << std::endl;
        // if there is boundary in the body we need to remove it to keep only the body
        // if (client.get_request()->is_boundary()){
        //     remove_boundary(client.get_request()->get_body(), client.get_request()->get_boundary());
        // }
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
        if (!client.get_server()) {
            std::cout << "Error: no server found" << std::endl;
            close_connection(client);
            return ;
        }
        // TODO: prepare response
        client.get_response()->set_server(client.get_server());
        client.get_response()->set_request(client.get_request());
        client.get_response()->create_response();
        //print body size
        // std::cout << "Body size: " << client.get_request()->get_body().length() << std::endl;
        // client.get_request()->print_request();
        // std::cout << "TOUT est OK" << std::endl;
        if (client.get_response()->_cgi_state == 1) {
            // add content to request body if request body length == 0
            if (client.get_request()->get_body().length() == 0) {
                client.get_request()->set_body(client.get_raw_request().substr(client.get_raw_request().find("\r\n\r\n") + 4));
            }
            // add to _current_writefds pipe_fd_in[1]
            FD_SET(client.get_response()->cgi->_pipe_fd_in[1], &_current_writefds);
            _max_fd = std::max(_max_fd, client.get_response()->cgi->_pipe_fd_in[1]);
            // add to _current_readfds pipe_fd_out[0]
            FD_SET(client.get_response()->cgi->_pipe_fd_out[0], &_current_readfds);
            _max_fd = std::max(_max_fd, client.get_response()->cgi->_pipe_fd_out[0]);
            // std::cout << "CGI" << std::endl;
        }
        // print full response
        // std::cout << "Full response: " << client.get_response()->get_full_response() << std::endl;
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
    int i = client.get_fd();
    close(i);

    // // if pipe open close it
    // if (client.get_response()->is_cgi()) {
    //     close(client.get_response()->cgi->_pipe_fd_in[0]);
    //     close(client.get_response()->cgi->_pipe_fd_in[1]);
    //     close(client.get_response()->cgi->_pipe_fd_out[0]);
    //     close(client.get_response()->cgi->_pipe_fd_out[1]);
    // }

    // clear allocated memory if there is any left
    if (client.get_request())
        delete client.get_request();
    if (client.get_response()->cgi)
        delete client.get_response()->cgi;
    if (client.get_response())
        delete client.get_response();
    // remove from _clients
    _clients.erase(i);
    // print in red and bold a client has been disconnected
    std::cout << "\033[1;31mClient disconnected\033[0m" << std::endl;
}

void    WebservManager::match_server(Client &client) {
    for (std::map<int, Server>::iterator it = _servers.begin(); it != _servers.end(); it++) {
        // // print server_name and port of it and client to see if it matches debug        
        // for (std::set<std::string>::iterator it2 = it->second.get_server_name().begin(); it2 != it->second.get_server_name().end(); it2++)
        //     std::cout << "server_name: " << *it2 << std::endl;
        // std::cout << "port: " << it->second.get_port() << std::endl;
        // std::cout << "client server_name: " << client.get_request()->get_server_name() << std::endl;
        // std::cout << "client port: " << client.get_request()->get_port() << std::endl;
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

void    WebservManager::send_cgi(Client &client) {
    std::cout << "Sending CGI to client" << std::endl;
    std::string &requestBody = client.get_request()->get_body();
    std::cout << "requestBody.size: " << requestBody.size() << std::endl;
    int send_ret = 0;
    if (requestBody.length() == 0) {}
    else if (requestBody.length() >= 42000)
        send_ret = write(client.get_response()->cgi->_pipe_fd_in[1], requestBody.c_str(), 42000);
    else
        send_ret = write(client.get_response()->cgi->_pipe_fd_in[1], requestBody.c_str(), requestBody.length());
    if (send_ret == -1) {
        std::cout << "Error: send failed" << std::endl;
        FD_CLR(client.get_response()->cgi->_pipe_fd_in[1], &_current_writefds);
        close(client.get_response()->cgi->_pipe_fd_in[1]);
        close(client.get_response()->cgi->_pipe_fd_out[1]);
        // close_connection(client);
    }
    else if (send_ret == 0 || (size_t)send_ret == requestBody.length()) {
        std::cout << "Done sending to CGI" << std::endl;
        if (FD_ISSET(client.get_response()->cgi->_pipe_fd_in[1], &_current_writefds)) {
            FD_CLR(client.get_response()->cgi->_pipe_fd_in[1], &_current_writefds);
            if (client.get_response()->cgi->_pipe_fd_in[1] == _max_fd) {
                _max_fd--;
            }
        }
        close(client.get_response()->cgi->_pipe_fd_in[1]);
        close(client.get_response()->cgi->_pipe_fd_out[1]);
        // client.get_response()->_cgi_state = 2;
    }
    else{
        std::cout << "send_ret: " << send_ret << std::endl;
        requestBody = requestBody.substr(send_ret);
        client.timer();
    }
}

void    WebservManager::read_cgi(Client &client) {
    char buffer[42000];
    int read_ret = 0;
    if ((read_ret = read(client.get_response()->cgi->_pipe_fd_out[0], buffer, 42000)) == -1) {
        FD_CLR(client.get_response()->cgi->_pipe_fd_out[0], &_current_readfds);
        close(client.get_response()->cgi->_pipe_fd_in[0]);
        close(client.get_response()->cgi->_pipe_fd_out[0]);
        std::cout << "Error: recv failed" << std::endl;
        client.get_response()->_cgi_state = 2;
        close_connection(client);
        return ;
    }
    else if (read_ret == 0) {
        int status;
        FD_CLR(client.get_response()->cgi->_pipe_fd_out[0], &_current_readfds);
        close(client.get_response()->cgi->_pipe_fd_out[0]);
        close(client.get_response()->cgi->_pipe_fd_in[0]);
        waitpid(client.get_response()->cgi->_pid, &status, 0);
        if (WEXITSTATUS(status) != 0) {
            std::cout << "Error: CGI failed" << std::endl;
            close_connection(client);
            return ;
        }
        client.get_response()->_cgi_state = 2;
    }
    else {
        // put buffer in response
        client.get_response()->get_full_response().append(buffer, read_ret);
        // std::cout << "Response: " << client.get_response()->get_full_response() << std::endl;
        client.timer();
    }
}

void    WebservManager::remove_boundary(std::string &body, std::string boundary) {
    std::string fileContent;
    size_t boundaryPos = 0;

    while ((boundaryPos = body.find(boundary, boundaryPos)) != std::string::npos) {
        // Trouver le début de la partie de données (après le boundary)
        size_t dataStartPos = body.find("\r\n\r\n", boundaryPos);

        if (dataStartPos != std::string::npos) {
            // Extraire la partie de données du corps
            fileContent += body.substr(dataStartPos + 4); // 4 pour sauter "\r\n\r\n"
        }

        // Recherchez le prochain boundary
        size_t nextBoundaryPos = body.find(boundary, boundaryPos + 1);
        
        if (nextBoundaryPos != std::string::npos) {
            boundaryPos = nextBoundaryPos;
        } else {
            break; // Arrêter la boucle si le prochain boundary n'est pas trouvé
        }
    }
    body = fileContent;
}