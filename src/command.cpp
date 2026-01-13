#include <cstdio>
#include <fmt/base.h>
#include <string>
#include <vector>

#include "structs.h"

using namespace std;

/// Splits a command up into a vec<string>
vector<string> split_command(string command) {
    vector<string> args;
    string arg;
    bool in_double_quotes = false;
    bool in_single_quotes = false;

    for (size_t i = 0; i < command.size(); i++) {
        char c = command[i];

        if (c == '"' && !in_single_quotes) {
            in_double_quotes = !in_double_quotes;
            continue;
        } else if (c == '\'' && !in_double_quotes) {
            in_single_quotes = !in_single_quotes;
            continue;
        }

        if (in_double_quotes || in_single_quotes) {
            arg.push_back(c);
        } else if (c == ' ') {
            if (!arg.empty()) {
                args.push_back(arg);
            }
            arg.clear();
        } else {
            arg.push_back(c);
        }
    }
    if (!arg.empty()) {
        args.push_back(arg);
    }

    return args;
}

void handle_command(BshContext &bsh_context) {
    vector<string> command_split = split_command(bsh_context.command);
    if (command_split.empty()) {
        return;
    }

    string exe = command_split[0];
    vector<string> args(command_split.begin() + 1, command_split.end());
    if (exe == "exit") {
        exit(0);
    } else if (exe == "pwd") {
        fmt::println("{}", bsh_context.current_dir);
    }
}
