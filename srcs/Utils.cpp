#include "../includes/Utils.hpp"

int stoi( std::string &s) {
    int i;
    std::istringstream(s) >> i;
    return i;
}

void invalid_config_file(void) {
    std::cerr << "Error: config file is invalid" << std::endl;
    exit(1);
}

std::vector<std::string>    split(std::string str, const std::string delimiter) {
    std::vector<std::string>    result;
    std::string::size_type      first = 0;
    std::string::size_type      last = str.find_first_of(delimiter, first);

    while (first != std::string::npos) {
        if (last != first)
            result.push_back(str.substr(first, last - first));
        first = str.find_first_not_of(delimiter, last);
        last = str.find_first_of(delimiter, first);
    }
    return result;
}

std::string  trim(std::string str) {
    std::string::size_type first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
        return std::string("");
    std::string::size_type last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

e_status line_checker(std::string line, std::stack<char> &bracket, e_status &status, ConfigParser &config) {
    line = trim(line);
    if (line.empty())
        return CONTINUE;

    std::vector<std::string>    tokens = split(line);
    int                         token_size = tokens.size();
    // std::cout << "line :" << line << std::endl;
    // std::cout << "status" << status << std::endl;
    if (config.get_status() == NONE) {
        if (token_size == 2 && tokens[0] == "server" && tokens[1] == "{") {
            bracket.push('{');
            return SERVER;
        }
    std::cout << "line :" << line << std::endl;
        invalid_config_file();
    }
    else if (config.get_status() == SERVER) {
        if (token_size == 3 && tokens[0] == "location" && tokens[2] == "{") {
            bracket.push('{');
            return LOCATION;
        }
        else if (token_size == 1 && tokens[0] == "}") {
    //std::cout << "HERE line: " << line << std::endl;
            bracket.pop();
            if (bracket.empty())
                return NONE;
        }
        else
            return SERVER;
    }
    else if (config.get_status() == LOCATION) {
        if (token_size == 1 && tokens[0] == "}") {
            bracket.pop();
            return SERVER;
        }
        else
            return LOCATION;
    }
    invalid_config_file();
    return ERROR;
}