#include <cstdio>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/format.h>
#include <string>
#include <system_error>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>

#include "structs.h"
#include "utils.h"

using namespace std;
using namespace fmt;

/// Splits a command up into a vec<string>
static vector<string> split_command(string &command) {
    vector<string> args;
    string arg;
    bool in_double_quotes = false;
    bool in_single_quotes = false;

    for (size_t i = 0; i < command.size(); i++) {
        char c = command[i];

        if (c == '"' && !in_single_quotes) {
            in_double_quotes = !in_double_quotes;
            continue;
        } else if (c == '\'' && !in_double_quotes) {
            in_single_quotes = !in_single_quotes;
            continue;
        }

        if (in_double_quotes || in_single_quotes) {
            arg.push_back(c);
        } else if (c == ' ') {
            if (!arg.empty()) {
                args.push_back(arg);
            }
            arg.clear();
        } else {
            arg.push_back(c);
        }
    }
    if (!arg.empty()) {
        args.push_back(arg);
    }

    return args;
}

void cd_command(BshContext &bsh_context, const vector<string> &args) {
    namespace fs = std::filesystem;

    fs::path target;

    if (args.empty()) {
        target = bsh_context.home_dir;
    } else {
        fs::path arg = args[0];
        target = arg.is_absolute()
            ? arg
            : fs::path(bsh_context.current_dir) / arg;
    }

    target = target.lexically_normal();

    std::error_code ec;
    fs::file_status st = fs::status(target, ec);

    if (ec) {
        println("cd: {}: {}", target.string(), ec.message());
        return;
    }

    if (!fs::exists(st)) {
        println("cd: no such file or directory: {}", target.string());
        return;
    }

    if (!fs::is_directory(st)) {
        println("cd: not a directory: {}", target.string());
        return;
    }

    if (::chdir(target.c_str()) != 0) {
        println("cd: permission denied: {}", target.string());
        return;
    }

    bsh_context.current_dir = fs::canonical(target).string();
    chdir(bsh_context.current_dir.c_str());
}

void run_command(const vector<string> &command_parts, string command) {
    pid_t pid = fork();
    if (pid == 0) {
        vector<char*> argv;
        for (const string &s : command_parts) {
            argv.push_back(const_cast<char*>(s.c_str()));
        }
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());

        string error_msg = ("failed to execute " + command);
        perror(error_msg.c_str());
        exit(1);
    } else if (pid != 0) {
        waitpid(pid, nullptr, 0);
    } else {
        perror("fork failed");
    }
}

void handle_command(BshContext &bsh_context) {
    vector<string> command_split = split_command(bsh_context.command);
    if (command_split.empty()) {
        return;
    }
    command_split = expand_home(command_split, bsh_context.home_dir);

    string exe = command_split[0];
    vector<string> args(command_split.begin() + 1, command_split.end());
    if (exe == "exit") {
        exit(0);
    } else if (exe == "pwd") {
        fmt::println("{}", bsh_context.current_dir);
    } else if (exe == "cd") {
        cd_command(bsh_context, args);
    } else {
        run_command(command_split, bsh_context.command);
    }
}
