#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

std::string replace_all(std::string s, const std::string& target, const std::string& replacement);
std::vector<std::string> expand_home(const std::vector<std::string> &args, const std::string &home);
std::vector<std::string> split_command(const std::string &command);

#endif
