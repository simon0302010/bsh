#include <fmt/base.h>
#include <string>
#include <vector>
#include <fmt/core.h>

#include "../utils/globals.h"

using namespace std;
using namespace fmt;

void history_help() {
    println("Usage: history <option>");
    println("Available options are [show, delete, help]");
}

int history_command(const vector<string> &args) {
    if (args.empty()) {
        history_help();
    } else if (args[0] == "help") {
        history_help();
    } else if (args[0] == "show") {
        for (const string &entry : history) {
            println("{}", entry);
        }
    } else if (args[0] == "delete") {
        clear_history_file = true;
    } else {
        println("Invalid option: {}", args[0]);
        history_help();
    }

    return 0;
}

