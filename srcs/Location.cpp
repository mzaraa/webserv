#include "../includes/Location.hpp"

Location::Location() {
    _root = "";
    _method.clear();
    _index = "";
    _autoindex = false;
    _redirect.clear();
    _cgi.clear();
}

Location::Location(const Location &src) {
    *this = src;
}

Location &Location::operator=(const Location &src) {
    if (this != &src) {
        _root = src._root;
        _method = src._method;
        _index = src._index;
        _autoindex = src._autoindex;
        _redirect = src._redirect;
        _cgi = src._cgi;
    }
    return *this;
}

Location::~Location() {}

/*      SETTERS       */
void    Location::set_root(std::string root) {
    _root = root;
}

void    Location::set_method(std::vector<std::string> methods) {
    // print methods to debug
    // for (size_t i = 1; i < methods.size(); i++)
    //     std::cout << methods[i] << std::endl;


    for (size_t i = 1; i < methods.size(); i++){
        if (methods[i] == "GET" || methods[i] == "POST" || methods[i] == "DELETE")
            _method.insert(methods[i]);
        else
            invalid_config_file();
    }
    // // print _method to debug
    // for (std::set<std::string>::iterator it = _method.begin(); it != _method.end(); it++) {
    //     std::cout << *it << std::endl;
    // }
}

void    Location::set_index(std::string index) {
    _index = index;
}

void    Location::set_autoindex(std::string str) {
    if (str == "on")
        _autoindex = true;
    else if (str == "off")
        _autoindex = false;
    else
        invalid_config_file();
        
}

void    Location::set_redirect(std::vector<std::string> tokens) {
    int error_code;
    std::string uri;

    error_code = stoi(tokens[1]);
    if (error_code < 300 || error_code > 399)
        invalid_config_file();
    uri = tokens[2];
    _redirect.insert(std::make_pair(error_code, uri));
}

void    Location::set_cgi(std::vector<std::string> cgi) {
    if (cgi.size() != 3)
        invalid_config_file();
    _cgi.insert(std::make_pair(cgi[1], cgi[2]));
}

/*      GETTERS       */
std::string                 Location::get_root() const {
    return _root;
}

std::set<std::string>       Location::get_method() const {
    return _method;
}

std::string                 Location::get_index() const {
    return _index;
}

bool                        Location::get_autoindex() const {
    return _autoindex;
}

std::map<int, std::string>  Location::get_redirect() const {
    return _redirect;
}

std::map<std::string, std::string> Location::get_cgi() const {
    return _cgi;
}

/*      DEBUG      */
void    Location::print_location() {
    if(_root != "")
        std::cout << "root: " << _root << std::endl;
    if(_method.size() != 0) {
        std::cout << "method: ";
        for (std::set<std::string>::iterator it = _method.begin(); it != _method.end(); it++) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
    }
    if(_index != "")
        std::cout << "index: " << _index << std::endl;
    if(_autoindex == true)
        std::cout << "autoindex: on" << std::endl;
    else if (_autoindex == false)
        std::cout << "autoindex: off" << std::endl;
    if(_redirect.size() != 0) {
        std::cout << "redirect: ";
        for (std::map<int, std::string>::iterator it = _redirect.begin(); it != _redirect.end(); it++) {
            std::cout << "code : " << it->first << " " << it->second << " "; 
        }
        std::cout << std::endl;
    }
}