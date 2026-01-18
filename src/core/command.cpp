#include <array>
#include <cstdio>
#include <cstdlib>
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <optional>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>

#include "context.h"
#include "../utils/utils.h"
#include "../builtins/cd.h"
#include "../builtins/about.h"
#include "../utils/globals.h"
#include "../builtins/history.h"

using namespace std;
using namespace fmt;

enum class FileMode {
    Overwrite,
    Append
};

struct WriterProperties {
    string file;
    FileMode mode;
};

struct Cmd {
    string stdin_stream;
    WriterProperties stdout_stream;
    WriterProperties stderr_stream;
    vector<string> args;
};

optional<vector<Cmd>> parse_redirection(const vector<vector<string>> &command_parts) {
    vector<Cmd> commands;
    for (const vector<string> &command : command_parts) {
        Cmd cmd;
        int i = 0;
        while (i < command.size()) {
            if (command[i] == "<") {
                if (i + 1 >= command.size()) {
                    println("missing file after <");
                    return nullopt;
                }
                cmd.stdin_stream = command[i + 1];
                i += 2;
            } else if (command[i] == ">") {
                if (i + 1 >= command.size()) {
                    println("missing file after >");
                    return nullopt;
                }
                cmd.stdout_stream = {command[i + 1], FileMode::Overwrite};
                i += 2;
            } else if (command[i] == ">>") {
                if (i + 1 >= command.size()) {
                    println("missing file after >>");
                    return nullopt;
                }
                cmd.stdout_stream = {command[i + 1], FileMode::Append};
                i += 2;
            } else if (command[i] == "2>") {
                if (i + 1 >= command.size()) {
                    println("missing file after 2>");
                    return nullopt;
                }
                cmd.stderr_stream = {command[i + 1], FileMode::Overwrite};
                i += 2;
            } else if (command[i] == "2>>") {
                if (i + 1 >= command.size()) {
                    println("missing file after 2>>");
                    return nullopt;
                }
                cmd.stderr_stream = {command[i + 1], FileMode::Append};
                i += 2;
            } else {
                cmd.args.push_back(command[i]);
                i += 1;
            }
        }

        if (cmd.args.empty()) {
            println("empty command");
            return nullopt;
        }

        commands.push_back(cmd);
    }

    return commands;
}

void run_command(const vector<string> &command_parts, const string &command) {
    vector<vector<string>> commands_pipes = split_vector(command_parts, "|");
    optional<vector<Cmd>> optional_commands = parse_redirection(commands_pipes);
    if (!optional_commands) {
        last_exit_code = 1;
        return;
    }
    vector<Cmd> commands = *optional_commands;

    int num_commands = commands.size();

    vector<array<int, 2>> pipes(num_commands - 1);
    for (int i = 0; i < num_commands - 1; i++) {
        pipe(pipes[i].data());
    }

    vector<pid_t> pids(num_commands);

    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();

        if (pids[i] == 0) {
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }

            if (i < num_commands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            vector<char*> c_args;
            for (const string &s : commands[i].args) {
                c_args.push_back(const_cast<char*>(s.c_str()));
            }
            c_args.push_back(nullptr);

            execvp(c_args[0], c_args.data());
            string error_msg = ("failed to execute \"" + commands[i].args[0] + "\"");
            perror(error_msg.c_str());
            exit(1);
        } else if (pids[i] < 0) {
            perror("fork failed");
        }
    }

    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    int status;
    bool exit_code_set = false;
    for (int i = 0; i < num_commands; i++) {
        waitpid(pids[i], &status, 0);
        if (!exit_code_set && status != 0) {
            if (WIFEXITED(status)) {
                last_exit_code = WEXITSTATUS(status);
                exit_code_set = true;
            }
        }
    }

    if (!exit_code_set) {
        last_exit_code = 0;
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
