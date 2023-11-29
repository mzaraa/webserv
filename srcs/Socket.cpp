# include "../includes/Socket.hpp"

Socket::Socket(Server &server) {
    _opt_flag = 1;
    _server = &server;
    _host = server.get_host();
    _port = server.get_port();
    _socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket_fd < 0) {
        std::cerr << "Error: socket creation failed" << std::endl;
        exit(1);
    }
    if (setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &_opt_flag, sizeof(_opt_flag)) < 0) {
        std::cerr << "Error: socket setsockopt failed" << std::endl;
        exit(1);
    }
    memset(&_addr, 0, sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(_port);
    _addr.sin_addr.s_addr = inet_addr(_host.c_str());
    // debug
    // std::cout << "host: " << _host << std::endl;
    // std::cout << "port: " << _port << std::endl;
    // std::cout << "socket_fd: " << _socket_fd << std::endl;
    // std::cout << "addr.sin_family: " << _addr.sin_family << std::endl;
    // std::cout << "addr.sin_port: " << _addr.sin_port << std::endl;
    // std::cout << "addr.sin_addr.s_addr: " << _addr.sin_addr.s_addr << std::endl;
    // end debug
    if (bind(_socket_fd, (struct sockaddr *)&_addr, sizeof(_addr)) < 0) {
        std::cerr << "Error: socket bind failed" << std::endl;
        exit(1);
    }
    // if (listen(_socket_fd, 10) < 0) {
    //     std::cerr << "Error: socket listen failed" << std::endl;
    //     exit(1);
    // }
}

Socket::Socket(const Socket &src) {
    *this = src;
}

Socket &Socket::operator=(const Socket &src) {
    if (this != &src) {
        _socket_fd = src._socket_fd;
        _port = src._port;
        _host = src._host;
        _addr = src._addr;
        _server = src._server;
        _opt_flag = src._opt_flag;
    }
    return *this;
}

Socket::~Socket() {
    //close(_socket_fd);
    //_socket_fd = -1;
}

/*      GETTERS       */
std::string         Socket::get_host() const {
    return _host;
}

int                 Socket::get_port() const {
    return _port;
}

int                 Socket::get_socket_fd() const {
    return _socket_fd;
}

struct sockaddr_in  Socket::get_addr() const {
    return _addr;
}

Server              *Socket::get_server() const {
    return _server;
}
