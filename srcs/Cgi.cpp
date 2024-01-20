#include "../includes/Cgi.hpp"

Cgi::Cgi() {
    _pid = -1;
    _pipe_fd_in[0] = -1;
    _pipe_fd_in[1] = -1;
    _pipe_fd_out[0] = -1;
    _pipe_fd_out[1] = -1;
    _status_code = NULL;
    _elapsed_time = 0;
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
        this->_status_code = src._status_code;
        this->_elapsed_time = src._elapsed_time;
        this->_env = src._env;
        this->_path = src._path;
        this->_av = src._av;
        this->_envp = src._envp;
    }
    return *this;
}

Cgi::~Cgi() {
}

void Cgi::set_cgi(Request *request, Server *server, std::string path) {
    set_env(request, server, path);
}

void Cgi::set_status_code(int *status_code) {
    _status_code = status_code;
}

void Cgi::set_env(Request *request, Server *server, std::string path) {
    std::map<std::string, std::string> headers = request->get_headers();
    Location &location = server->get_location()["/cgi"];
    std::string _exec_cgi = location.get_cgi()[".py"];
    _path = path;

    _env.push_back("REQUEST_METHOD=" + request->get_method());
    _env.push_back("CONTENT_TYPE=" + headers["Content-Type"]);
    _env.push_back("CONTENT_LENGTH=" + to_string(request->get_content_length()));
    _env.push_back("QUERY_STRING=" + request->get_query_string());
    _env.push_back("SCRIPT_NAME=" + request->get_path());
    _env.push_back("SERVER_PROTOCOL=" + request->get_version());
    _env.push_back("SERVER_SOFTWARE=webserv");
    _env.push_back("SERVER_NAME=" + request->get_server_name());
    _env.push_back("SERVER_PORT=" + to_string(request->get_port()));
    // _env.push_back("REMOTE_ADDR=" + request->get_host());
    // _env.push_back("PATH_INFO=" + request->get_path());
    // _env.push_back("PATH_TRANSLATED=" + request->get_path());

    // setup char** _envp
    _envp = new char*[_env.size() + 1];
    for (size_t i = 0; i < _env.size(); i++) {
        _envp[i] = strdup(_env[i].c_str());
    }
    _envp[_env.size()] = NULL;

    // setup char** _av
    _av = new char*[3];
    _av[0] = strdup(_exec_cgi.c_str());
    _av[1] = strdup(_path.c_str());
    _av[2] = NULL;
}

void Cgi::execute_cgi(int &status_code, std::string method) {
    const int timeout_sec = 5;
    if (_av[0] == NULL || _av[1] == NULL) {
        status_code = 500;
        return ;
    }
    gettimeofday(&_start_time, NULL);
    if (pipe(_pipe_fd_in) == -1) {
        status_code = 500;
        return ;
    }
    if (pipe(_pipe_fd_out) == -1) {
        status_code = 500;
        close(_pipe_fd_in[0]);
        close(_pipe_fd_in[1]);
        return ;
    }
    _pid = fork();
    if (_pid == -1) {
        status_code = 500;
        close(_pipe_fd_in[0]);
        close(_pipe_fd_in[1]);
        close(_pipe_fd_out[0]);
        close(_pipe_fd_out[1]);
        return ;
    }
    if (_pid == 0) {
        // child process
        dup2(_pipe_fd_in[0], 0);
        dup2(_pipe_fd_out[1], 1);
        close(_pipe_fd_in[1]);
        close(_pipe_fd_out[0]);
        close(_pipe_fd_in[0]);
        close(_pipe_fd_out[1]);
        if (execve(_av[0], _av, _envp) == -1) {
            std::cerr << "execve failed" << std::endl;
            exit(1);
        }
    }
    else if (_pid != 0){
        // parent process
        gettimeofday(&_end_time, NULL);
        _elapsed_time = (_end_time.tv_sec - _start_time.tv_sec) + (_end_time.tv_usec - _start_time.tv_usec) * 1e-6;
        int status;
        while (method == "GET" && waitpid(_pid, &status, WNOHANG) == 0 && _elapsed_time < timeout_sec) {
            gettimeofday(&_end_time, NULL);
            _elapsed_time = (_end_time.tv_sec - _start_time.tv_sec) + (_end_time.tv_usec - _start_time.tv_usec) * 1e-6;
        }
        if ((_elapsed_time >= timeout_sec || status != 0) && method == "GET") {
            std::cerr << "cgi timeout" << std::endl;
            *_status_code = 500;
            kill(_pid, SIGKILL);
            return ;
        }

        // make pipe non-blocking
        fcntl(_pipe_fd_in[1], F_SETFL, O_NONBLOCK);
        fcntl(_pipe_fd_out[0], F_SETFL, O_NONBLOCK);
    } // parent process
}