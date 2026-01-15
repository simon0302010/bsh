#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>
#include <vector>
#include <fstream>
#include <readline/readline.h>

#include "../utils/globals.h"
#include "../utils/utils.h"

using namespace std;

static vector<string> filtered_history;
static string cached_prefix;
static size_t history_size;

vector<string> get_matching_commands(const vector<string> &commands, const string &prefix) {
    vector<string> filtered;
    copy_if(commands.begin(), commands.end(), back_inserter(filtered), [&prefix](const string &command){
        return starts_with(command, prefix);
    });
    return filtered;
}

int arrow_down(int count, int key) {
    if (history.empty() || history_idx < 0) return 0;

    if (saved_line != cached_prefix || history.size() != history_size) {
        filtered_history = get_matching_commands(history, saved_line);
        cached_prefix = saved_line;
        history_size = history.size();
    }
    
    if (filtered_history.empty()) return 0;

    history_idx--;
    
    if (history_idx < 0) {
        // return to saved current line
        rl_replace_line(saved_line.c_str(), saved_line.size());
    } else if (history_idx < (int)filtered_history.size()) {
        string new_line = filtered_history[filtered_history.size() - 1 - history_idx];
        rl_replace_line(new_line.c_str(), new_line.size());
    }
    rl_point = rl_end;
    rl_redisplay();
    return 0;
}

int arrow_up(int count, int key) {
    if (history.empty()) return 0;

    // save current line
    if (history_idx < 0) {
        saved_line = string(rl_line_buffer);
    }

    if (saved_line != cached_prefix || history.size() != history_size) {
        filtered_history = get_matching_commands(history, saved_line);
        cached_prefix = saved_line;
        history_size = history.size();
    }
    
    if (filtered_history.empty()) return 0;

    if (history_idx < (int)filtered_history.size() - 1) {
        history_idx++;
        string new_line = filtered_history[filtered_history.size() - 1 - history_idx];
        rl_replace_line(new_line.c_str(), new_line.size());
        rl_point = rl_end;
        rl_redisplay();
    }
    return 0;
}

vector<string> read_history_file(const string &file_path) {
    ifstream history_file(file_path);
    if (!history_file) {
        return vector<string>();
    }

    vector<string> lines;
    string line;

    while (getline(history_file, line)) {
        lines.push_back(line);
    }

    return lines;
}