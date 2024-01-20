#pragma once

#include "Utils.hpp"

class Request;
class Location;
class Server;

class Cgi {
    public:
        pid_t   _pid;
        int     _pipe_fd_in[2];
        int     _pipe_fd_out[2];
        int*     _status_code;
        double  _elapsed_time;
        timeval _start_time;
        timeval _end_time;

        Cgi();
        Cgi(const Cgi &src);
        Cgi &operator=(const Cgi &src);
        ~Cgi();
        
        void    set_cgi(Request *request, Server *server, std::string path);
        void    execute_cgi(int &status_code, std::string method);
        void    set_env(Request *request, Server *server, std::string path);
        void    set_status_code(int *status_code);
    private:
        std::vector<std::string>    _env;
        std::string                 _path;
        char**                      _av;
        char**                      _envp; // envp is the same as _env but in char** envp
};
