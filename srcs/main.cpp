#include "../includes/ConfigParser.hpp"

static bool validation(std::vector<Server> &servers) {
    for (size_t i = 0; i < servers.size(); i++) {
        // if there is 2 server with the same server_name, host and port => false
        for (size_t j = i + 1; j < servers.size(); j++) {
            if (servers[i].get_server_name() == servers[j].get_server_name() &&
                servers[i].get_host() == servers[j].get_host() &&
                servers[i].get_port() == servers[j].get_port()) {
                return false;
            }
        }
    }
    return true;
}

int main (int ac, char *av[]) {
    if (ac != 2)     {
        std::cout << "Usage: ./webserv <config_file>" << std::endl;
        return 1;
    }
    std::vector<Server> servers;
    ConfigParser config_parser(av[1], servers);
    /**************************************************************************************************/
    // TODO: check in validation if there is requiered fields like host, port, in location root etc...
    /**************************************************************************************************/
    if (validation(servers) == false){
        std::cout << "Error: validation failed" << std::endl;
        return 1;
    }


    WebservManager webserv_manager(servers);
    webserv_manager.setup_servers_socket();
    webserv_manager.life_cycle();
    // std::vector<Socket> servers_socket;
    // setup_servers_socket(servers, &servers_socket);
    // life_cycle(servers_socket);
    // print servers
    // for (int i = 0; i < servers.size(); i++) {
    //     servers[i].print_server();
    // }
    return 0;
}