#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fmt/base.h>
#include <fmt/format.h>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>

#include "globals.h"
#include "utils.h"

using namespace std;

regex env_var_pattern(R"(\$\{[A-Za-z_][A-Za-z0-9_]*\}|\$[A-Za-z_][A-Za-z0-9_]*)");
const char* OPERATORS[] = {"|", ">", "<", ">>", "2>", "2>>"};

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

bool is_command_operator(const char &c) {
    for (const char* op : OPERATORS) {
        if (*op == c) {
            return true;
        }
    }
    return false;
}

// Returns operator length. 0 if it's not an operator.
int get_operator_length(const string &command, size_t i) {
    if (i >= command.size()) {
        return 0;
    }
    
    for (int len = 3; len >= 1; len--) {
        if (i + len <= command.size()) {
            string candidate = command.substr(i, len);
            for (const char* op : OPERATORS) {
                if (candidate == op) {
                    return len;
                }
            }
        }
    }
    
    return 0;
}

/// Splits a command up into a vec<string>
vector<Argument> split_command(const string &command) {
    vector<Argument> args;
    Argument arg;
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
            arg.text.push_back(c);
            arg.can_glob.push_back(false);
        } else {
            int op_len = get_operator_length(command, i);
            if (op_len > 0) {
                if (!arg.text.empty()) {
                    args.push_back(arg);
                    arg = {};
                }
                string operator_char = command.substr(i, op_len);
                args.push_back(Argument {operator_char, vector<bool>(operator_char.size(), false)});
                i += op_len - 1;
            } else if (c == '\\') {
                if (i + 1 < command.size()) {
                    arg.text.push_back(command[i + 1]);
                    arg.can_glob.push_back(false);
                    i++;
                } else {
                    arg.text.push_back(c);
                }
                arg.can_glob.push_back(false);
            } else if (c == ' ') {
                if (!arg.text.empty()) {
                    args.push_back(arg);
                    arg = {};
                }
            } else if (c == '#') {
                if (!arg.text.empty()) {
                    args.push_back(arg);
                }
                return args;
            } else {
                arg.text.push_back(c);
                arg.can_glob.push_back(true);
            }
        }
    }
    if (!arg.text.empty()) {
        args.push_back(arg);
    }
    
    /*fmt::println("Split command:");
    for (const string &arg : args) {
        fmt::println("{}", arg);
    }*/

    return args;
}

bool needs_glob(const Argument &a) {
    bool open = false;
    for (size_t i = 0; i < a.text.size(); ++i) {
        if (!a.can_glob[i]) continue;

        char c = a.text[i];
        if (c == '*' || c == '?' || c == '~') {
            return true;
        }
        if (c == '[') {
            open = true;
        }
        if (c == ']' && open) {
            return true;
        }
    }
    return false;
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

vector<vector<string>> split_vector(const vector<string> &items, const string &splitter) {
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

vector<char*> into_c_vec(const vector<string> &input) {
    vector<char*> output;
    for (const string &s : input) {
        output.push_back(const_cast<char*>(s.c_str()));
    }
    output.push_back(nullptr);
    return output;
}

vector<vector<string>> split_vector_deeper(const vector<string> &items, const char &splitter) {
    vector<vector<string>> splitted_vector;
    vector<string> current_vector;
    string current_string;
    
    for (string item : items) {
        for (char current_char : item) {
            if (current_char == splitter) {
                current_vector.push_back(current_string);
                splitted_vector.push_back(current_vector);
                current_string.clear();
                current_vector.clear();
            } else {
                current_string.push_back(current_char);
            }
        }
        current_vector.push_back(current_string);
        current_string.clear();
    }
    
    return splitted_vector;
}

vector<string> prepare_input(const string &s) {
    vector<string> splitted_string;
    string current_string;

    bool in_single_quotes = false;
    bool in_double_quotes = false;

    int i = 0;
    while (i < s.size()) {
        char c = s[i];

        if (c == '\'') {
            in_single_quotes = !in_single_quotes;
        } else if (c == '"') {
            in_double_quotes = !in_double_quotes;
        }

        if (c == '\\' && i + 1 < s.size() && s[i + 1] == '\n') {
            i += 2;
        } else if ((c == '\n' || c == ';') && !(in_single_quotes || in_double_quotes)) {
            splitted_string.push_back(current_string);
            current_string.clear();
            i++;
        } else {
            current_string.push_back(c);
            i++;
        }
    }
    splitted_string.push_back(current_string);

    /*fmt::println("Splitted:");
    for (const string &s : splitted_string) {
        fmt::println("{}", s);
    }*/

    return splitted_string;
}