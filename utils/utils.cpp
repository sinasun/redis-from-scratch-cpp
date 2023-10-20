#include "utils.h"

void die(const std::string& msg) {
    std::cerr << msg << std::endl;
    exit(1);
}

void msg(const std::string& msg) {
	std::cout << msg << std::endl;
}
