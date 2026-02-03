#include <fmt/base.h>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <filesystem>

#include "../utils/utils.h"
#include "utils/globals.h"

using namespace std;
using namespace fmt;
namespace fs = std::filesystem;

void get_path_names() {
    vector<string> environment = get_env();

    optional<string> path_var = nullopt;
    for (const string &var : environment) {
        if (starts_with(var, "PATH=")) {
            path_var = var.substr(5);
            break;
        }
    }

    if (!path_var) {
        return;
    }

    vector<string> directories = split_string(*path_var, ':');
    set<string> executables;

    for (const fs::path &dir : directories) {
        for (const auto &entry : fs::directory_iterator(dir)) {
            if (entry.is_regular_file() && (fs::status(entry).permissions() & fs::perms::owner_exec) != fs::perms::none) {
                executables.insert(entry.path().filename().string());
            }
        }
    }

    path_names.assign(executables.begin(), executables.end());
}