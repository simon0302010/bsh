#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <string>

inline std::vector<std::string> history;
inline int history_idx = -1;  // not navigating history when -1
inline std::string saved_line;     // saves current input when navigating history

#endif