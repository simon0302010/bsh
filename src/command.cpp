#include <cstdio>
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>

#include "structs.h"
#include "utils.h"
#include "commands/cd.h"
#include "commands/about.h"
#include "globals.h"

using namespace std;
using namespace fmt;

void run_command(const vector<string> &command_parts, string command) {
    pid_t pid = fork();
    if (pid == 0) {
        vector<char*> argv;
        for (const string &s : command_parts) {
            argv.push_back(const_cast<char*>(s.c_str()));
        }
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());

        string error_msg = ("failed to execute \"" + command + "\"");
        perror(error_msg.c_str());
        exit(1);
    } else if (pid != 0) {
        waitpid(pid, nullptr, 0);
    } else {
        perror("fork failed");
    }
}

void print_history() {
    for (string s : history) {
        println(s);
    }
}

bool handle_command(BshContext &bsh_context) {
    vector<string> command_split = split_command(bsh_context.command);
    if (command_split.empty()) {
        return true;
    }
    command_split = expand_home(command_split, bsh_context.home_dir);

    string exe = command_split[0];
    vector<string> args(command_split.begin() + 1, command_split.end());
    if (exe == "exit") {
        return false;
    } else if (exe == "pwd") {
        fmt::println("{}", bsh_context.current_dir);
    } else if (exe == "cd") {
        cd_command(bsh_context, args);
    } else if (exe == "about") {
        show_about();
    } else if (exe == "history") {
        print_history();
    } else {
        run_command(command_split, bsh_context.command);
    }
    return true;
}
