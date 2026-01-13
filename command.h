#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>

using namespace std;

vector<string> split_command(string command);
void handle_command(string command);

#endif
