#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "Utils.hpp"
# include "Server.hpp"
# include "Request.hpp"
# include "Response.hpp"

class Server;
class Request;
class Response;

class Client {
    public:
        Client();
        Client(Server &server);
        Client(const Client &src);
        Client &operator=(const Client &src);
        ~Client();

        void        timer();
        void        set_server(Server &server);
        /*      GETTERS       */
        int         get_fd() const;
        bool        has_received_header() const;
        bool        has_reiceved_request() const;
        std::string &get_raw_request();
        time_t      get_last_activity() const;
        Server      *get_server() const;
        Request     *get_request() const;
        Response    *get_response() const;


        class ClientDisconnectedException : public std::exception {
            virtual const char* what() const throw() {
                return "Client disconnected";
            }
        };

        class ClientReceiveRequestException : public std::exception {
            virtual const char* what() const throw() {
                return "Client receive request failed";
            }
        };

        class ClientSendResponseException : public std::exception {
            virtual const char* what() const throw() {
                return "Client send response failed";
            }
        };

        class NonBlockingException : public std::exception {
            virtual const char* what() const throw() {
                return "Fcntl failed";
            }
        };

    private:
        int         _fd;
        bool        _has_received_header;
        bool        _received_all_request;
        std::string _raw_request;
        time_t      _last_activity;
        Server      *_server;
        Request     *_request;
        Response    *_response;
};

#endif // CLIENT_HPP