#include "../includes/Server.hpp"

Server::Server() {
    _port = -1;
    _client_max_body_size = -1;
}

Server::Server(const Server &src) {
    *this = src;
}

Server &Server::operator=(const Server &src) {
    if (this != &src) {
        _port = src._port;
        _client_max_body_size = src._client_max_body_size;
        _server_name = src._server_name;
        _error_page = src._error_page;
        _location = src._location;
    }
    return *this;
}

Server::~Server() {}

/*      SETTERS       */
void    Server::set_port(std::string port) {
    if (port.length() > 5)
        invalid_config_file();
    for (size_t i = 0; i < port.length(); i++) {
        if (!std::isdigit(port[i]))
            invalid_config_file();
    }
    _port = stoi(port);
    if (_port < 1 || _port > 65535)
        invalid_config_file();
}

void    Server::set_client_max_body_size(std::string max_size) {
    //check if last character is M
    if (max_size[max_size.length() - 1] != 'M')
        invalid_config_file();
    //check if all characters are digits except last one
    for (size_t i = 0; i < max_size.length() - 1; i++) {
        if (!std::isdigit(max_size[i]))
            invalid_config_file();
    }
    _client_max_body_size = stoi(max_size);
}

void    Server::set_server_name(std::string server_name) {
    _server_name.insert(server_name);
}

void    Server::set_error_page(std::vector<std::string> tokens) {
    if (tokens[1].length() != 3)
        invalid_config_file();
    int error_code = stoi(tokens[1]);
    if (error_code < 100 || error_code > 599)
        invalid_config_file();
    std::string uri = tokens[2];
    _error_page.insert(std::make_pair(error_code, uri));
}

void    Server::set_location(std::string path) {
    _location.insert(std::make_pair(path, Location()));
}

/*      GETTERS       */

int                                     &Server::get_port() {
    return _port;
}

int                                     &Server::get_client_max_body_size() {
    return _client_max_body_size;
}

std::set<std::string>                   &Server::get_server_name() {
    return _server_name;
}

std::map<int, std::string>    &Server::get_error_page() {
    return _error_page;
}

Location                                &Server::get_location(const std::string &path) {
    return _location.at(path);
}

std::map<std::string, Location>   &Server::get_location(void) {
    return _location;
}

/*      DEBUG      */
void    Server::print_server() {
    //draw a visible banner to show debug info
    std::cout << "----------------------------------------" << std::endl;
    if (_port != -1)
        std::cout << "port: " << _port << std::endl;
    if (_client_max_body_size != -1)
        std::cout << "client_max_body_size: " << _client_max_body_size << std::endl;
    if (_server_name.size() != 0) {
        std::cout << "server_name: ";
        for (std::set<std::string>::iterator it = _server_name.begin(); it != _server_name.end(); it++)
            std::cout << *it << " ";
    }
    std::cout << std::endl;
    if (_error_page.size() != 0){
        std::cout << "error_page: ";
        for (std::map<int, std::string>::iterator it = _error_page.begin(); it != _error_page.end(); it++)
            std::cout << "code : " << it->first << " " << it->second << " "; 
    }
    std::cout << std::endl;
    if (_location.size() != 0) {
        std::cout << "location: " << std::endl;
        for (std::map<std::string, Location>::iterator it = _location.begin(); it != _location.end(); it++) {
            std::cout << "path: " << it->first << std::endl;
            it->second.print_location();
        }
    }
    std::cout << "----------------------------------------" << std::endl;
}