#include <cstdio>
#include <cstdlib>
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>
#include <algorithm>

#include "context.h"
#include "../utils/utils.h"
#include "../builtins/cd.h"
#include "../builtins/about.h"
#include "../utils/globals.h"
#include "../builtins/history.h"

using namespace std;
using namespace fmt;

void run_single_command(const vector<string> &command_parts, const string &command) {
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
        int status = 0;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            last_exit_code = WEXITSTATUS(status);
        }
    } else {
        perror("fork failed");
    }
}

void run_piped_command(const vector<vector<string>> &commands) {
    vector<char*> argv1;
    for (const string &s : commands[0]) {
        argv1.push_back(const_cast<char*>(s.c_str()));
    }
    argv1.push_back(nullptr);

    vector<char*> argv2;
    for (const string &s : commands[1]) {
        argv2.push_back(const_cast<char*>(s.c_str()));
    }
    argv2.push_back(nullptr);

    int pipedes[2];
    if (pipe(pipedes) != 0) {
        perror("failed to set up pipe");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        dup2(pipedes[1], STDOUT_FILENO);
        close(pipedes[0]);
        close(pipedes[1]);

        execvp(argv1[0], argv1.data());

        string error_msg = ("failed to execute");
        perror(error_msg.c_str());
        exit(1);
    } else if (pid1 > 0) {
        pid_t pid2 = fork();
        if (pid2 == 0) {
            dup2(pipedes[0], STDIN_FILENO);
            close(pipedes[0]);
            close(pipedes[1]);

            execvp(argv2[0], argv2.data());

            string error_msg = ("failed to execute");
            perror(error_msg.c_str());
            exit(1);
        } else if (pid2 > 0) {
            int status1 = 0;
            int status2 = 0;
            close(pipedes[0]);
            close(pipedes[1]);
            waitpid(pid1, &status1, 0);
            waitpid(pid2, &status2, 0);
            if (status1 != 0) {
                last_exit_code = status1;
            } else {
                last_exit_code = status2;
            }
        }
    }
}

void run_command(const vector<string> &command_parts, const string &command) {
    if (find(command_parts.begin(), command_parts.end(), "|") != command_parts.end()) {
        vector<vector<string>> splitted_vector = split_vector(command_parts, "|");
        if (splitted_vector.size() > 2) {
            println("support for multiple pipes is not yet implemented");
            return;
        }
        run_piped_command(splitted_vector);
    } else {
        run_single_command(command_parts, command);
    }
}

// TODO: export command
bool handle_command(BshContext &bsh_context) {
    vector<string> command = split_command(replace_env_vars(bsh_context.command));
    if (command.empty()) {
        return true;
    }
    command = expand_home(command, bsh_context.home_dir);

    string exe = command[0];
    vector<string> args(command.begin() + 1, command.end());
    if (exe == "exit") {
        return false;
    } else if (exe == "pwd") {
        fmt::println("{}", bsh_context.current_dir);
        last_exit_code = 0;
    } else if (exe == "cd") {
        last_exit_code = cd_command(bsh_context, args);
    } else if (exe == "about") {
        last_exit_code = show_about();
    } else if (exe == "history") {
        last_exit_code = history_command(args);
    } else {
        run_command(command, bsh_context.command);
    }
    return true;
}
