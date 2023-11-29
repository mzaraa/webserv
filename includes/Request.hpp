#ifndef REQUEST_HPP
# define REQUEST_HPP

# include "Utils.hpp"

class Request {
    public:
        Request();
        Request(const Request &src);
        Request &operator=(const Request &src);
        ~Request();

        void parse_request(std::string raw_request);
        void parse_first_line(std::string &first_line);
        void parse_header(std::string &header_line);
        void parse_body(std::istringstream &iss);
        void parse_chunked_body(std::istringstream &iss);
        void check_error();

        /*      SETTERS       */
        void set_remaining_body(int remaining_body);
        void set_full_body_received(bool full_body_received);
        void set_port(int port);
        void set_server_name(std::string server_name);
        void set_host(std::string host);

        /*      GETTERS       */
        std::string                 get_method() const;
        std::string                 get_path() const;
        std::string                 get_version() const;
        std::map<std::string, std::string>  get_headers() const;
        std::string                 get_body() const;
        std::size_t                 get_content_length() const;
        int                         get_remaining_body() const;
        bool                        has_body() const;
        bool                        is_chunked() const;
        bool                        full_body_received() const;
        int                         get_port() const;
        std::string                 get_host() const;
        std::string                 get_server_name() const;
        std::string                 get_boundary() const;
        bool                        is_boundary() const;
        int                         get_code_error() const;

        /*      DEBUG       */
        void print_request() const;
    private:
        std::string                         _method;
        std::string                         _path;
        std::string                         _query_string;
        std::string                         _fragment;
        std::string                         _version;
        std::map<std::string, std::string>  _headers;
        std::string                         _body;
        int                                 _port;
        std::string                         _host;
        std::string                         _server_name;
        std::string                         _boundary;
        bool                                _is_boundary;
        int                                 _content_length;
        int                                 _remaining_body;
        std::size_t                         _chunked_size;
        bool                                _has_body;
        bool                                _is_chunked;
        bool                                _full_body_received;
        int                                 _code_error;
};

#endif // REQUEST_HPP