#include "../includes/ConfigParser.hpp"

int main (int ac, char *av[]) {
    if (ac != 2)     {
        std::cout << "Usage: ./webserv <config_file>" << std::endl;
        return 1;
    }
    std::vector<Server> servers;
    ConfigParser config_parser(av[1], servers);
    
    // print servers
    for (int i = 0; i < servers.size(); i++) {
        servers[i].print_server();
    }
    return 0;
}