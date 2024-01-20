#include "../includes/ConfigParser.hpp"

static bool validation(std::vector<Server> &servers) {
    for (size_t i = 0; i < servers.size(); i++) {
        // for all location in server, check if allowed methods is empty, if yes => then add GET POST DELETE
        for (std::map<std::string, Location>::iterator it = servers[i].get_location().begin(); it != servers[i].get_location().end(); it++) {
            // print location key
            // std::cout << "***********************************************************************************" << std::endl;
            // std::cout << "LOCATION KEY : " << it->first << std::endl;
            if (it->second.get_method().empty()) {
                // std::cout << "METHODS EMPTY for location : " << it->first << std::endl;
                std::vector<std::string> methods;
                methods.push_back("GET");
                methods.push_back("POST");
                methods.push_back("DELETE");
                it->second.set_method(methods);
            }
            // else {
            //     std::cout << "METHODS FIND for location : " << it->first << std::endl;
            //     std::set<std::string> temp = it->second.get_method();
            //     for (std::set<std::string>::iterator it2 = temp.begin(); it2 != temp.end(); it2++) {
            //         std::cout << *it2 << std::endl;
            //         std::cout << "HERE" << std::endl;
            //     }
            // }
        }
    }
    for (size_t i = 0; i < servers.size(); i++) {
        // if there is 2 server with the same server_name, host and port => false
        if (servers[i].get_host().empty() || servers[i].get_port() == -1)
            return false;
        if (servers[i].get_location().count("/cgi") == 0 || servers[i].get_location()["/cgi"].get_cgi().empty() || !servers[i].get_location()["/cgi"].get_cgi().count(".py") || \
            servers[i].get_location()["/cgi"].get_cgi()[".py"].empty())
            return false;
        if (servers[i].get_location().count("/") == 0 || servers[i].get_location()["/"].get_root().empty())
            return false;
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

    // for (size_t i = 0; i < servers.size(); i++) {
    //     servers[i].print_server();
    // }

    WebservManager webserv_manager(servers);
    webserv_manager.setup_servers_socket();
    webserv_manager.life_cycle();


    // std::vector<Socket> servers_socket;
    // setup_servers_socket(servers, &servers_socket);
    // life_cycle(servers_socket);
    // print servers
    return 0;
}