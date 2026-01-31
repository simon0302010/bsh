#ifndef GLOBALS_H
#define GLOBALS_H

#include <unordered_map>
#include <vector>
#include <string>

inline std::vector<std::string> history;
inline int history_idx = -1;  // not navigating history when -1
inline std::string saved_line;     // saves current input when navigating history
inline int last_exit_code = 0;
inline bool clear_history_file = false;
inline long last_command_duration = 0; // time last command took to execute in milliseconds
inline long last_command_timestamp = 0; // unix timestamp of time last command finished
inline std::pmr::unordered_map<std::string, std::string> environment_vars;
inline std::vector<std::string> current_vars;

#endif