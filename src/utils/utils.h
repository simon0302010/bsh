#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <optional>

struct Argument {
    std::string text;
    std::vector<bool> can_glob;
};

void replace_all(std::string& s, const std::string& target, const std::string& replacement);
std::string replace_all_return(std::string s, const std::string& target, const std::string& replacement);
std::vector<std::string> expand_home(const std::vector<std::string> &args, const std::string &home);
std::vector<Argument> split_command(const std::string &command);
std::string replace_env_vars(const std::string &command);
bool starts_with(const std::string &str, const std::string &prefix);
std::string get_exit_code_string();
std::vector<std::vector<std::string>> split_vector(const std::vector<std::string> &items, const std::string &splitter);
std::vector<char*> into_c_vec(const std::vector<std::string> &input);
std::vector<std::string> prepare_input(const std::string &s);
bool needs_glob(const Argument &a);
std::string get_prompt_symbol();
std::string format_duration(long duration);
std::vector<std::string> split_string(const std::string &s, char splitter);
void set_env(const std::string &key, const std::string &value);
const char * get_var(const std::string &key);
void get_env();

// Trim from the start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// Trim from the end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

#endif