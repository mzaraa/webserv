#pragma once

#include "Utils.hpp"

class Request;
class Location;

class Cgi {
    public:
        pid_t   _pid;
        int     _pipe_fd_in[2];
        int     _pipe_fd_out[2];

        Cgi();
        Cgi(const Cgi &src);
        Cgi &operator=(const Cgi &src);
        ~Cgi();
        
    private:
        std::map<std::string, std::string> _env;
        std::string                         _path;
        char**                              _av;
        char**                              _envp; // envp is the same as _env but in char** envp
};
