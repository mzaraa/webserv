#include "../includes/Cgi.hpp"

Cgi::Cgi() {
    _pid = -1;
    _pipe_fd_in[0] = -1;
    _pipe_fd_in[1] = -1;
    _pipe_fd_out[0] = -1;
    _pipe_fd_out[1] = -1;
    _envp = NULL;
    _av = NULL;
}

Cgi::Cgi(const Cgi &src) {
    *this = src;
}

Cgi &Cgi::operator=(const Cgi &src) {
    if (this != &src) {
        this->_pid = src._pid;
        this->_pipe_fd_in[0] = src._pipe_fd_in[0];
        this->_pipe_fd_in[1] = src._pipe_fd_in[1];
        this->_pipe_fd_out[0] = src._pipe_fd_out[0];
        this->_pipe_fd_out[1] = src._pipe_fd_out[1];
        this->_env = src._env;
        this->_path = src._path;
        this->_av = src._av;
        this->_envp = src._envp;
    }
    return *this;
}

Cgi::~Cgi() {
}
