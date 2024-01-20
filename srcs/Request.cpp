#include "../includes/Request.hpp"

Request::Request() {
    _method = "";
    _path = "";
    _query_string = "";
    _fragment = "";
    _version = "";
    _headers = std::map<std::string, std::string>();
    _body = "";
    _port = 0;
    _host = "";
    _server_name = "";
    _content_length = 0;
    _remaining_body = 0;
    _chunked_size = 0;
    _has_body = false;
    _is_chunked = false;
    _full_body_received = true;
    _code_error = 0;
    _boundary = "";
    _is_boundary = false;
}

Request::Request(const Request &src) {
    *this = src;
}

Request &Request::operator=(const Request &src) {
    if (this != &src) {
        _method = src._method;
        _path = src._path;
        _version = src._version;
        _headers = src._headers;
        _body = src._body;
        _content_length = src._content_length;
        _has_body = src._has_body;
        _is_chunked = src._is_chunked;
        _full_body_received = src._full_body_received;
        _remaining_body = src._remaining_body;
        _port = src._port;
        _host = src._host;
        _server_name = src._server_name;
        _code_error = src._code_error;
        _boundary = src._boundary;
        _is_boundary = src._is_boundary;
        _query_string = src._query_string;
        _fragment = src._fragment;
        _chunked_size = src._chunked_size;
    }
    return *this;
}

Request::~Request() {
}

void Request::parse_request(std::string raw_request) {
    // std::cout << "Parsing request..." << std::endl;
    std::istringstream iss(raw_request);
    std::string line;
    std::getline(iss, line);
    line = line.substr(0, line.find("\r"));
    
    //std::cout << "First line parser..." << std::endl;
    //std::cout << "First line: " << line << std::endl;
    std::vector<std::string> tokens = split(line, " ");
    parse_first_line(line);
    
    //std::cout << "Header parser.." << std::endl;
    while (std::getline(iss, line) && !line.empty() && line != "\r") {
        line = line.substr(0, line.find("\r"));
        parse_header(line);
    }

    //    // if (_headers.find("Content-Type") != _headers.end()) {
    //     std::string type = _headers["Content-Type"];
    //     if (type.find("multipart/form-data") != std::string::npos) {
    //         size_t boundaryPos = _headers["Content-Type"].find("boundary=");
    //         if (boundaryPos != std::string::npos) {
    //             _boundary = _headers["Content-Type"].substr(boundaryPos + 9);
    //             _is_boundary = true;
    //         }
    //     }
    // }std::cout << "Request parsed" << std::endl;
    // tellg() returns -1 if EOF is reached
    if (iss.tellg() != -1) {
        parse_body(iss);
    }
    if (_headers.find("Host") != _headers.end()){
        std::string host = _headers["Host"];
        if (host.find(":") != std::string::npos) {
            _host = host.substr(0, host.find(":"));
            if (!is_ip_address(_host)){
                _server_name = _host;
                _host = "";
            }
            _port = std::atoi(host.substr(host.find(":") + 1).c_str());
        }
        
    }
    //std::cout << "Request parsed - now error checking" << std::endl;
    check_error();
    //std::cout << "Parsing DOne" << std::endl;
    //print body
    // std::cout << "Body: " << _body << std::endl;
}

void Request::check_error(){
    // if (_method != "GET" || _method != "POST" || _method != "DELETE" ){
    //     _code_error = 501;
    // }
    // method should be GET, POST or DELETE
    if (_method != "GET" && _method != "POST" && _method != "DELETE"){
        _code_error = 501;
    }
    //if firt charatere of path is not '/' so error
    else if (_path[0] != '/'){
        _code_error = 400;
    }
    // check if path have only valid characters
    else if (_path.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~:/?#[]@!$&'()*+,;=") != std::string::npos){
        _code_error = 400;
    }
    // check if uri is not too long
    else if (_path.size() > 2048){
        _code_error = 414;
    }
    // check if version is valid
    else if (_version != "HTTP/1.1"){
        _code_error = 505;
    }
    //check if there is no traveral path
    else if (_path.find("../") != std::string::npos){
        _code_error = 400;
    }

}

void Request::parse_body(std::istringstream &iss) {
    if (_headers.find("Content-Type") != _headers.end()) {
        std::string type = _headers["Content-Type"];
        if (type.find("multipart/form-data") != std::string::npos) {
            size_t boundaryPos = _headers["Content-Type"].find("boundary=");
            if (boundaryPos != std::string::npos) {
                _boundary = _headers["Content-Type"].substr(boundaryPos + 9);
                _is_boundary = true;
            }
        }
    }
    if (_headers.find("Content-Length") != _headers.end()) {
        _content_length = std::atoi(_headers["Content-Length"].c_str());
        char buffer[42001];
        //read from iss into buffer
        iss.read(buffer, 42001);
        // buffer[_content_length] = '\0';
        _body.append(buffer, iss.gcount());
        _remaining_body = _content_length - _body.size();
        // std::cout << "Remaining body: " << _remaining_body << std::endl;
        if (_remaining_body > 0) {
            _full_body_received = false;
        }
    }
    if (_headers.find("Transfer-Encoding") != _headers.end()) {
        _is_chunked = true;
        parse_chunked_body(iss);
        return ;
    }
}

void Request::parse_chunked_body(std::istringstream &iss) {
    std::ostringstream oss;
    std::string line;

    while (std::getline(iss, line) && !line.empty()) {
        _chunked_size = 0;
        std::istringstream(line) >> std::hex >> _chunked_size;
        if (_chunked_size == 0) {
            break ;
        }
        char buffer[_chunked_size + 1];
        iss.read(buffer, _chunked_size);
        buffer[_chunked_size] = '\0';
        oss << buffer;
    }
    _body = oss.str();
}

void Request::parse_first_line(std::string &first_line) {
    std::istringstream iss(first_line);
    iss >> _method >> _path >> _version;
    // std::cout << "Method: " << _method << std::endl;
    // std::cout << "Path: " << _path << std::endl;
    // std::cout << "Version: " << _version << std::endl;
    if (_path.find("?") != std::string::npos) {
        _query_string = _path.substr(_path.find("?") + 1);
        _path = _path.substr(0, _path.find("?"));
    }
    if (_path.find("#") != std::string::npos) {
        _fragment = _path.substr(_path.find("#") + 1);
        _path = _path.substr(0, _path.find("#"));
    }
}

void Request::parse_header(std::string &header_line) {
    std::size_t pos = header_line.find(":");
    if (pos != std::string::npos) {
        std::string key = header_line.substr(0, pos);
        // remove space after ':'
        std::string value = header_line.substr(pos + 1);
        while (std::isspace(value[0]) && value.size() > 0)
            value = value.substr(1);
        _headers[key] = value;
        if (key == "Transfer-Encoding" && value.find("chunked") != std::string::npos){
            _is_chunked = true;
        }
    }
}


std::string Request::get_method() const {
    return _method;
}

std::string Request::get_path() const {
    return _path;
}

std::string Request::get_version() const {
    return _version;
}

std::map<std::string, std::string> Request::get_headers() const {
    return _headers;
}

std::string& Request::get_body() {
    return _body;
}

std::size_t Request::get_content_length() const {
    return _content_length;
}

bool Request::has_body() const {
    return _has_body;
}

bool Request::is_chunked() const {
    return _is_chunked;
}

bool Request::full_body_received() const {
    return _full_body_received;
}

int Request::get_remaining_body() const {
    return _remaining_body;
}

int Request::get_port() const {
    return _port;
}

std::string Request::get_host() const {
    return _host;
}

std::string Request::get_server_name() const {
    return _server_name;
}

std::string Request::get_boundary() const {
    return _boundary;
}

bool Request::is_boundary() const {
    return _is_boundary;
}

int Request::get_code_error() const {
    // std::cout << "HERE" << _body.size() << std::endl;
    return _code_error;
}

std::string Request::get_query_string() const {
    return _query_string;
}


/*      SETTERS       */
void Request::set_remaining_body(int remaining_body) {
    _remaining_body = remaining_body;
}

void Request::set_full_body_received(bool full_body_received) {
    _full_body_received = full_body_received;
}

void Request::set_port(int port) {
    _port = port;
}

void Request::set_server_name(std::string server_name) {
    _server_name = server_name;
}

void Request::set_host(std::string host) {
    _host = host;
}

void Request::set_body(std::string body) {
    _body = body;
}

/*      DEBUG       */
void Request::print_request() const {
    std::cout << "Method: " << _method << std::endl;
    std::cout << "Path: " << _path << std::endl;
    std::cout << "Version: " << _version << std::endl;
    std::cout << "Headers: " << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "Body: " << _body << std::endl;

    // std::cout << "Content-Length: " << _content_length << std::endl;
    // std::cout << "Remaining-Body: " << _remaining_body << std::endl;
    // std::cout << "Has-Body: " << _has_body << std::endl;
    // std::cout << "Is-Chunked: " << _is_chunked << std::endl;
    // std::cout << "Full-Body-Received: " << _full_body_received << std::endl;
    // std::cout << "Port: " << _port << std::endl;
    // std::cout << "Host: " << _host << std::endl;
    // std::cout << "Server-Name: " << _server_name << std::endl;
}