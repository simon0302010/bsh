#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>

struct BshContext {
    std::string current_dir;
    std::string command;
    std::string home_dir;
    std::string username;
    std::string hostname;
};

#endif
