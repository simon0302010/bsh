#include <fmt/base.h>
#include <string>
#include <vector>
#include <fmt/core.h>

#include "../utils/globals.h"
#include "core/context.h"

using namespace std;
using namespace fmt;

void history_help() {
    println("Usage: history <option>");
    println("Available options are [show, delete, size, help]");
}

int history_command(BshContext &bsh_context) {
    if (bsh_context.args.empty()) {
        history_help();
    } else if (bsh_context.args[0] == "help") {
        history_help();
    } else if (bsh_context.args[0] == "show") {
        for (const string &entry : history) {
            println("{}", entry);
        }
    } else if (bsh_context.args[0] == "size") {
        println("{} command(s)", history.size());
    } else if (bsh_context.args[0] == "delete") {
        clear_history_file = true;
    } else {
        println("Invalid option: {}", bsh_context.args[0]);
        history_help();
    }

    return 0;
}

