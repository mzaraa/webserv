#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Utils.hpp"
# include "Cgi.hpp"

class Server;
class Request;
class Cgi;

class Response {
    public:
        Response();
        Response(const Response &src);
        Response &operator=(const Response &src);
        ~Response();

        void            create_response();
        void            build_response();
        void            build_error_response();
        void            create_html_error_page(int code);
        bool            manage_location();
        std::string     construct_path(std::string root, std::string path);
        bool            manage_cgi(std::string &best_location);
        bool            put_file_in_body();
        bool            generate_auto_index();
        void            complete_response();

        /*      SETTERS       */
        void set_status_line();
        // void set_headers(std::string headers);
        void set_body(std::string body, int size);
        void set_server(Server *server);
        void set_request(Request *request);
        void set_remaining_body(int remaining_body);
        void set_error(int error_code);
        void set_cgi_status(bool status);

        /*      GETTERS       */
        std::string get_status_code(int code) const;
        std::string get_mime_type(std::string extension) const;
        std::string &get_full_response();
        bool is_cgi() const;
        int  get_error_code() const;

        /*      DEBUG       */
        void print_response() const;

        Cgi     *cgi;
        int     _cgi_state;
    private:
        std::map<int, std::string> _status_codes;
        std::map<std::string, std::string> _mime_types;
        std::string _full_response;
        std::string _status_line;
        std::string _headers;
        std::string _body;
        int         _error_code;
        std::string _redirect_url;
        std::string _file;
        bool        _is_cgi;
        bool        _auto_index;
        Server      *_server;
        Request     *_request;
};

#endif // RESPONSE_HPP