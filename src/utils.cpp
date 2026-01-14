#include <cstddef>
#include <string>
#include <vector>

using namespace std;

string replace_all(string s, const string& target, const string& replacement) {
    size_t pos = 0;
    while ((pos = s.find(target, pos)) != string::npos) {
        s.replace(pos, target.length(), replacement);
        pos += replacement.length();
    }
    return s;
}

vector<string> expand_home(const vector<string> &args, const string &home) {
    vector<string> new_args;
    for (size_t i = 0; i < args.size(); i++) {
        string part = args[i];
        if ((!part.empty() && part[0] == '~') && (part.size() == 1 || part[1] == '/')) {
            part.replace(0, 1, home);
        }
        new_args.push_back(part);
    }
    return new_args;
}

/// Splits a command up into a vec<string>
vector<string> split_command(const string &command) {
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
