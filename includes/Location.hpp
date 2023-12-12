#ifndef LOCATION_HPP
# define LOCATION_HPP

# include "Utils.hpp"

class Location {
    public:
        Location();
        Location(const Location &src);
        Location &operator=(const Location &src);
        ~Location();

        /*      SETTERS       */
        void    set_root(std::string root);
        void    set_method(std::vector<std::string> methods);
        void    set_index(std::string index);
        void    set_autoindex(std::string str);
        void    set_redirect(std::vector<std::string> tokens);
        void    set_cgi(std::vector<std::string> cgi);

        /*      GETTERS       */
        std::string                 get_root() const;
        std::set<std::string>       get_method() const;
        std::string                 get_index() const;
        bool                        get_autoindex() const;
        std::map<int, std::string>  get_redirect() const;
        std::map<std::string, std::string> get_cgi() const;

        /*      DEBUG      */
        void    print_location();

    private:
        std::string                 _root;
        std::set<std::string>       _method;
        std::string                 _index;
        bool                        _autoindex;
        std::map<int, std::string>  _redirect;
        std::map<std::string, std::string> _cgi;

};

# endif /* LOCATION_HPP */