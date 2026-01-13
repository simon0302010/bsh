#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <vector>

#include "structs.h"

using namespace std;

vector<string> split_command(string command);
void handle_command(BshContext &bsh_context);

#endif
