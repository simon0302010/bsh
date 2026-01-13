#include <string>
#include <vector>

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

void handle_command(string command) {
    vector<string> command_split = split_command(command);
    if (command == "exit") {
        exit(0);
    }
}
