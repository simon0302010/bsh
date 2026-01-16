#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fmt/format.h>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>

#include "globals.h"

using namespace std;

regex env_var_pattern(R"(\$\{[A-Za-z_][A-Za-z0-9_]*\}|\$[A-Za-z_][A-Za-z0-9_]*)");

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

string replace_env_vars(const string &command) {    
    string result = command;
    smatch match;
    
    while (regex_search(result, match, env_var_pattern)) {
        string found_string = match.str();
        string clean_var = found_string;
        
        clean_var.erase(remove(clean_var.begin(), clean_var.end(), '{'), clean_var.end());
        clean_var.erase(remove(clean_var.begin(), clean_var.end(), '}'), clean_var.end());
        clean_var.erase(remove(clean_var.begin(), clean_var.end(), '$'), clean_var.end());
        
        const char* env_value = getenv(clean_var.c_str());
        string replacement;
        
        if (env_value != nullptr) {
            // check if already inside quotes
            size_t pos = match.position();
            bool already_quoted = false;
            
            if (pos > 0 && (result[pos-1] == '"' || result[pos-1] == '\'')) {
                already_quoted = true;
            }
            
            if (already_quoted) {
                replacement = string(env_value);
            } else {
                replacement = "\"" + string(env_value) + "\"";
            }
        } else {
            replacement = found_string;
        }
        
        result.replace(match.position(), match.length(), replacement);
    }
    
    return result;
}

bool starts_with(const string &str, const string &prefix) {
    if (prefix.size() == 0) {
        return true;
    }
    return str.compare(0, prefix.length(), prefix) == 0;
}

string get_exit_code_string() {
    if (last_exit_code != 0) {
        return fmt::format(" ({})", last_exit_code);
    }
    return "";
}

vector<vector<string>> split_vector(const vector<string> items, string splitter) {
    vector<vector<string>> splitted_vector;
    vector<string> current_vector;
    for (string item : items) {
        if (item == splitter) {
            splitted_vector.push_back(current_vector);
            current_vector.clear();
        } else {
            current_vector.push_back(item);
        }
    }
    splitted_vector.push_back(current_vector);

    return splitted_vector;
}