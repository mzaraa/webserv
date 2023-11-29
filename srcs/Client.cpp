#include "../includes/Client.hpp"

Client::Client() {
}

Client::Client(Server &server) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    _fd = accept(server.get_socket_fd(), (struct sockaddr *)&client_addr, &client_addr_len);
    if (_fd == -1) {
        std::cout << "accept() failed" << std::endl;
        return ;
    }
    if (fcntl(_fd, F_SETFL, O_NONBLOCK) < 0) {
        std::cerr << "Error: fcntl failed" << std::endl;
        throw NonBlockingException();
    }
    _has_received_header = false;
    _received_all_request = false;
    _raw_request = "";
    _last_activity = time(NULL);
    _request = new Request();
    _response = new Response();
    _server = NULL;
}

Client::Client(const Client &src) {
    *this = src;
}

Client &Client::operator=(const Client &src) {
    if (this != &src) {
        _fd = src._fd;
        _has_received_header = src._has_received_header;
        _received_all_request = src._received_all_request;
        _raw_request = src._raw_request;
        _last_activity = src._last_activity;
        _request = src._request;
        _response = src._response;
        _server = src._server;
    }
    return *this;
}

Client::~Client() {
    //close(_fd);
    //_fd = -1;
}

void        Client::timer() {
    _last_activity = time(NULL);
}

int         Client::get_fd() const {
    return _fd;
}

bool        Client::has_received_header() const {
    return _has_received_header;
}

bool        Client::has_reiceved_request() const {
    return _received_all_request;
}

std::string &Client::get_raw_request() {
    return _raw_request;
}

time_t      Client::get_last_activity() const {
    return _last_activity;
}

Request     *Client::get_request() const {
    return _request;
}

Response    *Client::get_response() const {
    return _response;
}

Server      *Client::get_server() const {
    return _server;
}

void        Client::set_server(Server &server) {
    _server = &server;
}