#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <string>

inline std::vector<std::string> history;
inline int history_idx = -1;  // not navigating history when -1
inline std::string saved_line;     // saves current input when navigating history
inline int last_exit_code = 0;
inline bool clear_history_file = false;

#endif