#include "../includes/Utils.hpp"

void replace_char(std::string& str, char old_c, char new_c) {
    size_t length = str.length();

    for (size_t i = 0; i < length; ++i) {
        if (str[i] == old_c) {
            str[i] = new_c;
        }
    }
}

void to_upper(std::string& str) {
    size_t length = str.length();

    for (size_t i = 0; i < length; ++i) {
        str[i] = static_cast<char>(toupper(str[i]));
    }
}


std::vector<std::string>    get_files_in_directory(const std::string &path) {
    std::vector<std::string>    files;
    DIR                         *dp;
    struct dirent               *dirp;

    if ((dp = opendir(path.c_str())) == NULL) {
        std::cerr << "Error: cannot open directory" << std::endl;
        exit(1);
    }
    // loop through all the files and directories within directory dir and push them into vector files
    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_name[0] != '.')
            files.push_back(dirp->d_name);
    }
    closedir(dp);
    return files;
}

bool is_directory(const std::string &path) {
    struct stat s;
    if (stat(path.c_str(), &s) == 0) {
        if (s.st_mode & S_IFDIR)
            return true;
    }
    return false;
}

// check if path is a file
bool is_file(const std::string &path) {
    struct stat s;
    if (stat(path.c_str(), &s) == 0) {
        if (s.st_mode & S_IFREG)
            return true;
    }
    return false;
}

std::vector<std::string> get_files_in_directory(std::string dir) {
    std::vector<std::string>    files;
    DIR                         *dp;
    struct dirent               *dirp;

    if ((dp = opendir(dir.c_str())) == NULL) {
        std::cerr << "Error: cannot open directory" << std::endl;
        exit(1);
    }
    // loop through all the files and directories within directory dir and push them into vector files
    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_name[0] != '.')
            files.push_back(dirp->d_name);
    }
    closedir(dp);
    return files;

}

bool is_ip_address(std::string str) {
    std::vector<std::string>    tokens = split(str, ".");
    int                         token_size = tokens.size();

    if (token_size != 4)
        return false;
    for (int i = 0; i < token_size; i++) {
        if (tokens[i].empty())
            return false;
        for (size_t j = 0; j < tokens[i].size(); j++) {
            if (!isdigit(tokens[i][j]))
                return false;
        }
        int num = stoi(tokens[i]);
        if (num < 0 || num > 255)
            return false;
    }
    return true;
}

int stoi( std::string &s) {
    int i;
    std::istringstream(s) >> i;
    return i;
}

std::string itos(int i) {
    std::stringstream s;
    s << i;
    return s.str();
}

bool exist(std::string path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool file_exists(const std::string &path) {
    bool exists = access(path.c_str(), F_OK) == 0;
    return exists;
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

e_status line_checker(std::string line, std::stack<char> &bracket, ConfigParser &config) {
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