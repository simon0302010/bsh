#ifndef UTILS_HISTORY_H
#define UTILS_HISTORY_H

#include <string>
#include <vector>

int arrow_down(int count, int key);
int arrow_up(int count, int key);
std::vector<std::string> read_history_file(const std::string &file_path);

#endif