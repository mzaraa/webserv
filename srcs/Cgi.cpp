#include "../includes/Cgi.hpp"

Cgi::Cgi() {
}

Cgi::Cgi(const Cgi &src) {
    *this = src;
}

Cgi &Cgi::operator=(const Cgi &src) {
    if (this != &src) {
    }
    return *this;
}

Cgi::~Cgi() {
}
