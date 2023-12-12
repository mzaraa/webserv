#include "../includes/ConfigParser.hpp"

ConfigParser::ConfigParser(std::string config_file, std::vector<Server> &servers): _server_count(0), _status(NONE) {
    struct stat buffer;

    if (!stat(config_file.c_str(), &buffer) && S_ISDIR(buffer.st_mode)) {
        std::cerr << "Error: cannot find config file" << std::endl;
        exit(1);
    }
    std::ifstream ifs(config_file.c_str());
    if (!ifs.is_open()) {
        std::cerr << "Error: cannot open config file" << std::endl;
        exit(1);
    }
    parse_config_file(ifs, servers);
}

ConfigParser::ConfigParser(const ConfigParser &src) {
    *this = src;
}

ConfigParser &ConfigParser::operator=(const ConfigParser &src) {
    if (this != &src) {
        _server_count = src._server_count;
        _status = src._status;

    }
    return *this;
}

ConfigParser::~ConfigParser() {}

void    ConfigParser::parse_config_file(std::ifstream &ifs, std::vector<Server> &servers) {
    std::stack<char>    bracket;
    std::string         line;
    int                 is_cgi = 0;

    while (std::getline(ifs, line)) {
        _status = line_checker(line, bracket, *this);
        if (_status == CONTINUE || _status == NONE) {
            _status = NONE;
            continue;
        }
        Server server;
        while(_status == SERVER) {
            if (std::getline(ifs, line)) {
                _status = line_checker(line, bracket, *this);
                if (_status == CONTINUE) {
                    _status = SERVER;
                    continue;
                }
                if (_status == NONE)
                    break;
                if (_status == LOCATION) {
                    std::vector<std::string> tokens = split(line);
                    // if (tokens[1][tokens[1].length() - 1] != '/')
                    //     tokens[1] += '/';
                    server.set_location(tokens[1]);
                    Location &location = server.get_location(tokens[1]);
                    if (line.find("cgi") != std::string::npos) {
                        is_cgi = 1;
                    }
                    while (_status == LOCATION) {
                        getline(ifs, line);
                        _status = line_checker(line, bracket, *this);
                        if (_status == SERVER)
                            break;
                        if (_status == CONTINUE) {
                            _status = LOCATION;
                            continue;
                        }
                        std::vector<std::string> tokens = split(trim(line));
                        if (is_cgi) 
                            setup_location_cgi(tokens, location);
                        else
                            setup_location(tokens, location);
                    }
                    if (_status == SERVER)
                    {
                        if (is_cgi)
                            is_cgi = 0;
                        continue;
                    }
                }
                setup_server(split(line), server);
                //std::cout << "line :" << line << std::endl;
            }
        }
        servers.push_back(server);
    }
}

void    ConfigParser::setup_server(std::vector<std::string> tokens, Server &server) {
    if (tokens[0] == "server_name" && tokens.size() == 2)
        server.set_server_name(tokens[1]);
    else if (tokens[0] == "port" && tokens.size() == 2)
        server.set_port(tokens[1]);
    else if (tokens[0] == "client_max_body_size" && tokens.size() == 2)
        server.set_client_max_body_size(tokens[1]);
    else if (tokens[0] == "error_page" && tokens.size() == 3)
        server.set_error_page(tokens);
    else if (tokens[0] == "host" && tokens.size() == 2)
        server.set_host(tokens[1]);
    else
        invalid_config_file();
}

void    ConfigParser::setup_location(std::vector<std::string> tokens, Location &location) {
    if (tokens[0] == "root" && tokens.size() == 2)
        location.set_root(tokens[1]);
    else if (tokens[0] == "index" && tokens.size() == 2)
        location.set_index(tokens[1]);
    else if (tokens[0] == "autoindex" && tokens.size() == 2)
        location.set_autoindex(tokens[1]);
    else if (tokens[0] == "allow_methods" && tokens.size() >= 2)
        location.set_method(tokens);
    else if (tokens[0] == "redirect" && tokens.size() == 3)
        location.set_redirect(tokens);
    else
        invalid_config_file();
}

void    ConfigParser::setup_location_cgi(std::vector<std::string> tokens, Location &location) {
    if (tokens[0] == "root" && tokens.size() == 2)
        location.set_root(tokens[1]);
    else if (tokens[0] == "index" && tokens.size() == 2)
        location.set_index(tokens[1]);
    else if (tokens[0] == "allow_methods" && tokens.size() >= 2)
        location.set_method(tokens);
    else if (tokens[0] == "cgi" && tokens.size() == 3)
        location.set_cgi(tokens);
    else
        invalid_config_file();
}

/*      SETTERS       */
void    ConfigParser::set_server_count(int server_count) {
    _server_count = server_count;
}

void    ConfigParser::set_status(e_status status) {
    _status = status;
}

/*      GETTERS       */
int         ConfigParser::get_server_count() const {
    return _server_count;
}

e_status    ConfigParser::get_status() const {
    return _status;
}
