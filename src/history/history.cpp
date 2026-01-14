#include <string>
#include <vector>
#include <fstream>
#include <readline/readline.h>

#include "../utils/globals.h"

using namespace std;

int arrow_down(int count, int key) {
    if (history.empty() || history_idx < 0) return 0;

    history_idx--;
    
    if (history_idx < 0) {
        // return to saved current line
        rl_replace_line(saved_line.c_str(), saved_line.size());
    } else {
        string new_line = history[history.size() - 1 - history_idx];
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

    if (history_idx < (int)history.size() - 1) {
        history_idx++;
        string new_line = history[history.size() - 1 - history_idx];
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

    while (std::getline(history_file, line)) {
        lines.push_back(line);
    }

    return lines;
}