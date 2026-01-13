#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

using namespace std;

string replace_all(string s, const string& target, const string& replacement);
vector<string> expand_home(const vector<string> &args, const string &home);

#endif
