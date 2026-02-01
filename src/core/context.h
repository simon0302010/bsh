#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <vector>

struct BshContext {
    std::string current_dir;
    std::string command;
    std::string home_dir;
    std::string username;
    std::string hostname;
    std::vector<std::string> args;
    std::string config_path;
};

#endif
