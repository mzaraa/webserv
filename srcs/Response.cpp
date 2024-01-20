#include "../includes/Response.hpp"

Response::Response() {
    cgi = NULL;
    _cgi_state = 0;
    _mime_types = std::map<std::string, std::string>();
    _status_codes = std::map<int, std::string>();
    _server = NULL;
    _request = NULL;
    _error_code = 0;
    _status_line = "";
    _headers = "";
    _body = "";
    _full_response = "";
    _redirect_url = "";
    _file = "";
    _is_cgi = false;
    _auto_index = false;

   // MIME types
    _mime_types[".html"] = "text/html";
    _mime_types[".css"] = "text/css";
    _mime_types[".js"] = "text/javascript";
    _mime_types[".jpg"] = "image/jpeg";
    _mime_types[".jpeg"] = "image/jpeg";
    _mime_types[".png"] = "image/png";
    _mime_types[".gif"] = "image/gif";
    _mime_types[".svg"] = "image/svg+xml";
    _mime_types[".ico"] = "image/x-icon";
    _mime_types[".mp3"] = "audio/mpeg";
    _mime_types[".mp4"] = "video/mp4";
    _mime_types[".ttf"] = "font/ttf";
    _mime_types[".otf"] = "font/otf";
    _mime_types[".wasm"] = "application/wasm";
    _mime_types[".json"] = "application/json";
    _mime_types[".xml"] = "application/xml";
    _mime_types[".pdf"] = "application/pdf";
    _mime_types[".zip"] = "application/zip";
    _mime_types[".tar"] = "application/x-tar";
    _mime_types[".gz"] = "application/gzip";
    _mime_types[".7z"] = "application/x-7z-compressed";
    _mime_types[".rar"] = "application/x-rar-compressed";
    _mime_types[".csv"] = "text/csv";
    _mime_types[".txt"] = "text/plain";
    _mime_types[".rtf"] = "application/rtf";
    _mime_types[".doc"] = "application/msword";
    _mime_types[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";

        // codes and messages 2xx
    _status_codes[200] = "OK";
    _status_codes[201] = "Created";
    _status_codes[202] = "Accepted";
    _status_codes[203] = "Non-Authoritative Information";
    _status_codes[204] = "No Content";
    _status_codes[205] = "Reset Content";
    _status_codes[206] = "Partial Content";
    // codes and messages 3xx
    _status_codes[300] = "Multiple Choices";
    _status_codes[301] = "Moved Permanently";
    _status_codes[302] = "Found";
    _status_codes[303] = "See Other";
    _status_codes[304] = "Not Modified";
    _status_codes[305] = "Use Proxy";
    // codes and messages 4xx
    _status_codes[400] = "Bad Request";
    _status_codes[401] = "Unauthorized";
    _status_codes[402] = "Payment Required";
    _status_codes[403] = "Forbidden";
    _status_codes[404] = "Not Found";
    _status_codes[405] = "Method Not Allowed";
    _status_codes[406] = "Not Acceptable";
    _status_codes[407] = "Proxy Authentication Required";
    _status_codes[408] = "Request Timeout";
    _status_codes[409] = "Conflict";
    _status_codes[410] = "Gone";
    _status_codes[411] = "Length Required";
    _status_codes[412] = "Precondition Failed";
    _status_codes[413] = "Payload Too Large";
    _status_codes[414] = "URI Too Long";
    _status_codes[415] = "Unsupported Media Type";
    _status_codes[416] = "Range Not Satisfiable";
    _status_codes[417] = "Expectation Failed";
    // codes and messages 5xx
    _status_codes[500] = "Internal Server Error";
    _status_codes[501] = "Not Implemented";
    _status_codes[502] = "Bad Gateway";
    _status_codes[503] = "Service Unavailable";
    _status_codes[504] = "Gateway Timeout";
    _status_codes[505] = "HTTP Version Not Supported";
}

Response::Response(const Response &src) {
    *this = src;
}

Response &Response::operator=(const Response &src) {
    if (this != &src) {
        this->cgi = src.cgi;
        this->_cgi_state = src._cgi_state;
        this->_status_codes = src._status_codes;
        this->_mime_types = src._mime_types;
        this->_full_response = src._full_response;
        this->_status_line = src._status_line;
        this->_headers = src._headers;
        this->_body = src._body;
        this->_error_code = src._error_code;
        this->_redirect_url = src._redirect_url;
        this->_server = src._server;
        this->_request = src._request;
        this->_file = src._file;
        this->_is_cgi = src._is_cgi;
        this->_auto_index = src._auto_index;
    }
    return (*this);
}

Response::~Response() {
}

void Response::create_response() {
    build_response();
}

void Response::build_response() {
    // std::cout << "BUILD RESPONSE 1" << std::endl;
    if (_server == NULL) {
        std::cout << "ERROR: server is NULL" << std::endl;
        exit(1) ;
    }
    if (_request->get_code_error() != 0) {
        _error_code = _request->get_code_error();
        // std::cout << "ERROR CODE: " << _error_code << std::endl;
        build_error_response();
    }
    else if (_request->get_body().size() > _server->get_client_max_body_size()) {
        _error_code = 413;
        build_error_response();
    }
    else if (manage_location() == false) {
        build_error_response();
    }
    if (_is_cgi){
        std::cout << "IS CGI" << std::endl;
        if (_error_code !=500){
            _cgi_state = 1;
            return ;
        }
        else
            build_error_response();
    }
    else if (_auto_index) {
        if (generate_auto_index() == false) {
            build_error_response();
        }
        else
            _error_code = 200;
    }
    // std::cout << "BUILD RESPONSE 5" << std::endl;
    complete_response();
}

void Response::complete_response() {
    // set status line
    set_status_line();
    _full_response.append(_status_line);
    // set headers
    if (_file.find_last_of('.') != std::string::npos && _error_code == 200) {
        std::string extension = _file.substr(_file.find_last_of('.'));
        _headers.append("Content-Type: ").append(get_mime_type(extension)).append("\r\n");
    }
    else
        _headers.append("Content-Type: text/html\r\n");
    _headers.append("Content-Length: ").append(itos(_body.size())).append("\r\n");
    if (_request->get_headers()["Connection"] == "keep-alive")
        _headers.append("Connection: keep-alive\r\n");
    else
        _headers.append("Connection: close\r\n");
    _headers.append("Server: webserv/1.0.0 (Unix) (Ubuntu/Linux)\r\n");
    if (_redirect_url.empty() == false) {
        _headers.append("Location: ").append(_redirect_url).append("\r\n");
    }
    _headers.append("\r\n");
    _full_response.append(_headers);
    //std::cout << "FULL RESPONSE" << std::endl;
    if (_request->get_method() == "GET" || _error_code != 200) {
        _full_response.append(_body);
    }
    // // print full response
    // std::cout << "FULL RESPONSE: " << std::endl;
    // std::cout << _full_response << std::endl;
}

bool Response::generate_auto_index() {
    // read directory and put all files in string tmp
    std::string tmp;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(_file.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            tmp.append(ent->d_name);
            tmp.append("\n");
        }
        closedir(dir);
    }
    else {
        _error_code = 500;
        return (false);
    }
    // create html page
    std::string auto_index_page;
    auto_index_page.append("<html>\n<head>\n<title>Index of ");
    auto_index_page.append(_request->get_path());
    auto_index_page.append("</title>\n</head>\n<body>\n<h1>Index of ");
    auto_index_page.append(_request->get_path());
    auto_index_page.append("</h1><hr><pre>\n");
    auto_index_page.append(tmp);
    auto_index_page.append("</pre><hr>\n</body>\n</html>");
    _body = auto_index_page;
    return (true);
}

bool Response::manage_location() {
    std::string best_location = "";
    // std::cout << "REQUEST PATH: " << _request->get_path() << std::endl;
    for (std::map<std::string, Location>::iterator it = _server->get_location().begin(); it != _server->get_location().end(); it++) {
        // std::cout << "LOCATION: " << it->first << std::endl;
        if (_request->get_path().find(it->first) == 0) {
            if (it->first == "/" || it->first.length() == _request->get_path().length() || _request->get_path()[it->first.length()] == '/') {
                if (best_location.length() < it->first.length()) {
                    best_location = it->first;
                }
            }
        }
    }
    // std::cout << "--------------------------------------------" << std::endl;
    // std::cout << "HEEEEE BEST LOCATION: " << best_location << std::endl;
    // if no location found for the request
    if (best_location.empty()) {
        _file = construct_path(_server->get_location()["/"].get_root(), _request->get_path());
        // std::cout << "BEST LOCATION EMPTY -- _file_root: " << _file << std::endl;
        // if _file is a directory
        if (is_directory(_file)) {
            // if there is no '/' at the end of the path
            if (_file[_file.length() - 1] != '/') {
                // std::cout << "HERE" << std::endl;
                _error_code = 301;
                _redirect_url = _request->get_path() + "/";
                return (false);
            }
            _file.append(_server->get_location()["/"].get_index());
            // std::cout << "BEST LOCATION EMPTY -- _file_index: " << _file << std::endl;
            if (!exist(_file)) {
                // std::cout << "HERE 2" << std::endl;
                _error_code = 403;
                return (false);
            }
            if (is_directory(_file)) {
                _redirect_url = construct_path(_request->get_path(), _server->get_location()["/"].get_index());
                if (_redirect_url[_redirect_url.length() - 1] != '/') {
                    _redirect_url.append("/");
                }
                _error_code = 301;
                return (false);
            }
        }
            
    }
    else {
        // std::cout << "BEST LOCATION NOT EMPTY: " << best_location << std::endl;
        Location location = _server->get_location()[best_location];
        std::set<std::string> tmp = location.get_method();
        // std::cout << "HERE DELETE DEBUG 1" << std::endl;
        // check if method of the request is allowed in the location
        if (tmp.find(_request->get_method()) == tmp.end()) {
            _error_code = 405;
            return (false);
        }
        // std::cout << "HERE DELETE DEBUG 2" << std::endl;
        // check if there is return in the location
        std::map<int, std::string> tmp_map = location.get_redirect();
        std::map<int, std::string>::iterator it = tmp_map.begin();
        if (it != tmp_map.end()) {
            _error_code = it->first;
            _redirect_url = it->second;
            std::cout << "REDIRECT URL: " << _redirect_url << std::endl;
            // if (_redirect_url[0] != '/') {
            //     _redirect_url = "/" + _redirect_url;
            // }
            return (false);
        }
        // check if path is the cgi directory
        if (best_location.find("cgi") != std::string::npos) { 
            if (manage_cgi(best_location))
                return (true);
            else
                return (false);
        }
        // if not cgi, check if path is a directory
        _file = construct_path(location.get_root(), _request->get_path().substr(best_location.length()));
        // std::cout << "FILE: " << _file << std::endl;
        // std::cout << "----------------------" << std::endl;
        if (is_directory(_file)) {
            // if file is a directory and there is no '/' at the end of the path, add it and redirect to the new url
            if (_file[_file.length() - 1] != '/') {
                _error_code = 301;
                _redirect_url = _request->get_path() + "/";
                return (false);
            }
            // if it's a directory and there is no index in the location, return 403 forbidden, else add the index file
            if (location.get_index().empty() && location.get_autoindex() == false) {
                // std::cout << "NO INDEX" << std::endl;
                _error_code = 403;
                return (false);
            }
            else if (!location.get_index().empty() && location.get_autoindex() == false) {
                _file.append(location.get_index()); 
            }
            else 
                _file.append("doesntexist");
            // if file doesn't exist, check for autoindex then return 403 forbidden if autoindex is false 
            if (!exist(_file)) {
                // std::cout << "FILE DOESN'T EXIST" << std::endl;
                if (location.get_autoindex() == false) {
                    std::cout << "AUTOINDEX FALSE" << std::endl;
                    _error_code = 403;
                    return (false);
                }
                else {
                    // std::cout << "AUTOINDEX TRUE" << std::endl;
                    _file.erase(_file.find_last_of('/') + 1);
                    _auto_index = true;
                    return (true);
                }
            }
            // if file exist, check if it's a directory to redirect to the new url
            if (is_directory(_file)) {
                if (location.get_index().empty()){
                    std::cout << "COOUCOU" << std::endl;
                    _error_code = 403;
                    return (false);
                }
                else{
                    _redirect_url = construct_path(_request->get_path(), location.get_index());
                }
                if (_redirect_url[_redirect_url.length() - 1] != '/') {
                    _redirect_url.append("/");
                }
                _error_code = 301;
                return (false);
            }
        }
    }
    if (_is_cgi || _error_code)
        return (true);
    if (_request->get_method() == "GET") {
        if (put_file_in_body() == false) {
            return (false);
        }
    }
    else if (_request->get_method() == "POST") {
        if (exist(_file)) {
            _error_code = 202;
            return (true);
        }
        std::ofstream file(_file.c_str(), std::ios::binary);
        if (!file.good()) {
            _error_code = 404;
            return (false);
        }
        if (_request->is_boundary()) {
            // show boundary
            std::cout << "BOUNDARY: " << _request->get_boundary() << std::endl;
            _error_code = 400;
            return (false);
        }
        else{
            // write body in file
            file.write(_request->get_body().c_str(), _request->get_body().size());
        }
    }
    else if (_request->get_method() == "DELETE") {
        // try if file exist
        if (!exist(_file)) {
            _error_code = 404;
            return (false);
        }
        // now remove the file
        if (remove(_file.c_str()) == -1) {
            std::cout << "LOLOLOL" << std::endl;
            _error_code = 403;
            return (false);
        }
    }
    _error_code = 200;
    return (true);
}

bool Response::put_file_in_body() {
    std::ifstream file(_file.c_str());
    if (!file.good()) {
        _error_code = 404;
        return (false);
    }
    // put the full file in _body
    std::string line;
    while (getline(file, line)) {
        _body.append(line);
        _body.append("\n");
    }
    return (true);
}

bool    Response::manage_cgi(std::string &best_location) {
    std::string cgi_path;
    std::string cgi_extension;

    cgi_path = _request->get_path();
    // std::cout << "CGI PATH: " << cgi_path << std::endl;
    if ( cgi_path == "/cgi/") {
        cgi_path.append(_server->get_location()[best_location].get_index());
    }
    else if (cgi_path == "/cgi") {
        cgi_path.append("/").append(_server->get_location()[best_location].get_index());
    }
    // cgi_path[0] == '/' remove it
    if (cgi_path[0] == '/') {
        cgi_path = cgi_path.substr(1);
    }

    // std::cout << "CGI PATH: " << cgi_path << std::endl;
    // check allowed methods
    if (_server->get_location()[best_location].get_method().find(_request->get_method()) == _server->get_location()[best_location].get_method().end()) {
        _error_code = 405;
        return (false);
    }
    cgi_extension = cgi_path.substr(cgi_path.find_last_of('.'));
    if (cgi_extension.empty()) {
        _error_code = 501;
        return (false);
    }
    if (cgi_extension != ".py" ) {
        _error_code = 501;
        return (false);
    }
    // check if it's a file
    if (!is_file(cgi_path)) {
        _error_code = 404;
        return (false);
    }
    // check if we have rights to execute the file
    if (access(cgi_path.c_str(), X_OK) == -1) {
        std::cout << "ERROR: " << strerror(errno) << std::endl;
        _error_code = 403;
        return (false);
    }
    _is_cgi = true;

    // // print headers request
    // std::cout << "--------------------------------------------------------------" << std::endl;
    // std::cout << "REQUEST HEADERS" << std::endl;
    // std::map<std::string, std::string> headers = _request->get_headers();
    // for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); it++) {
    //     std::cout << it->first << ": " << it->second << std::endl;
    // }
    // std::cout << "--------------------------------------------------------------" << std::endl;

    cgi = new Cgi();
    cgi->set_cgi(_request, _server, cgi_path);
    cgi->set_status_code(&_error_code);
    cgi->execute_cgi(_error_code, _request->get_method());
    return (true);
}

std::string Response::construct_path(std::string root, std::string path) {
    if (root[root.length() - 1] != '/' && path[0] != '/') {
        root.append("/");
    }
    if (root[root.length() - 1] == '/' && path[0] == '/') {
        path = path.substr(1);
    }
    return (root + path);
}

void Response::build_error_response() {
    if (_request->get_method() == "DELETE" || _request->get_method() == "POST" || _server->get_error_page().empty() || (_server->get_error_page().find(_error_code) == _server->get_error_page().end())) {
        // std::cout << "PEUT PAD DELETE DONC CREATION HTML ERROR PAGE" << std::endl;
        create_html_error_page(_error_code);
    }
    else {
        if (_error_code >= 400 && _error_code < 500) {
            // send redirect url
            _redirect_url = _server->get_error_page()[_error_code];
            //check if url is relative or absolute
            _error_code = 302;
            if (_redirect_url[0] != '/' || (_redirect_url[0] != '.' && _redirect_url[1] != '/')) {
                _redirect_url = "/" + _redirect_url;
            }
            _file = construct_path(_server->get_location()["/"].get_root(), _server->get_error_page()[_error_code]);
            std::ifstream file(_file.c_str());
            if (!file.good()) {
                create_html_error_page(_error_code);
            }
            else {
                // put the full file in _body
                std::string line;
                while (getline(file, line)) {
                    _body.append(line);
                    _body.append("\n");
                }
            }
        }
        
    }
}

void Response::create_html_error_page(int code) {
    std::string error_page;
    error_page.append("<html>\n<head>\n<title>");
    error_page.append(itos(code));
    error_page.append(" ");
    error_page.append(get_status_code(code));
    error_page.append("</title>\n</head>\n<body>\n<center><h1>");
    error_page.append(itos(code));
    error_page.append(" ");
    error_page.append(get_status_code(code));
    error_page.append("</h1></center>\n<hr><center>webserv/1.0.0 (Unix) (Ubuntu/Linux)</center>\n</body>\n</html>");
    _body = error_page;
}

/*      SETTERS       */
void Response::set_status_line() {
    // _status_line = "HTTP/1.1 " + itos(_code) + " " + get_status_code(_code) + "\r\n";
    _status_line.append("HTTP/1.1 ").append(itos(_error_code)).append(" ").append(get_status_code(_error_code)).append("\r\n");
    
}

void Response::set_server(Server *server) {
    _server = server;
}

void Response::set_request(Request *request) {
    _request = request;
}

void Response::set_remaining_body(int remaining_body) {
    _full_response = _full_response.substr(remaining_body);
}

void Response::set_error(int error_code) {
    _error_code = error_code;
    _body = "";
    _full_response = "";
    build_error_response();
    complete_response();
}

void Response::set_cgi_status(bool status) {
    _is_cgi = status;
}

void Response::set_body(std::string body, int size) {
    _body.append(body, size);
}

/*     GETTERS       */
std::string Response::get_status_code(int code) const {
    std::string status_code;
    std::map<int, std::string>::const_iterator it = _status_codes.find(code);
    if (it != _status_codes.end()) {
        status_code = it->second;
    }
    return (status_code);
}

std::string Response::get_mime_type(std::string extension) const {
    std::string mime_type;
    std::map<std::string, std::string>::const_iterator it = _mime_types.find(extension);
    if (it != _mime_types.end()) {
        mime_type = it->second;
    }
    return (mime_type);
}

bool Response::is_cgi() const {
    return (_is_cgi);
}

std::string &Response::get_full_response() {
    return (_full_response);
}

int Response::get_error_code() const {
    return _error_code;
}
