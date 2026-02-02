#include <array>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <glob.h>
#include <optional>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <sys/wait.h>
#include <fcntl.h>

#include "context.h"
#include "../utils/utils.h"
#include "../builtins/cd.h"
#include "../builtins/about.h"
#include "../utils/globals.h"
#include "../builtins/history.h"
#include "../builtins/vars.h"

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
    optional<string> stdin_stream = nullopt;
    optional<WriterProperties> stdout_stream = nullopt;
    optional<WriterProperties> stderr_stream = nullopt;
    vector<string> args;
};

// TODO: better parsing
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
        if (pipe(pipes[i].data()) != 0) {
            println("failed to create pipe {}", i + 1);
            return;
        }
    }

    vector<pid_t> pids(num_commands);

    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();

        if (pids[i] == 0) {
            // warnings
            if (commands[i].stdout_stream && i < num_commands - 1) {
                println("warning: stdout to file takes priority over piped stdout");
            }
            if (commands[i].stdin_stream && i > 0) {
                println("stdin from file takes priority over stdin from pipe");
            }

            // stdout into file or pipe
            if (commands[i].stdout_stream) {
                int flags = (commands[i].stdout_stream->mode == FileMode::Overwrite) 
                    ? O_WRONLY | O_CREAT | O_TRUNC 
                    : O_WRONLY | O_CREAT | O_APPEND;
                int filefd = open(commands[i].stdout_stream->file.c_str(), flags, 0644);
                if (filefd < 0) {
                    string error_msg = "failed to open " + commands[i].stdout_stream->file;
                    perror(error_msg.c_str());
                    exit(1);
                }
                dup2(filefd, STDOUT_FILENO);
                close(filefd);
            } else if (i < num_commands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // stdin into file or pipe
            if (commands[i].stdin_stream) {
                int filefd = open(commands[i].stdin_stream->c_str(), O_RDONLY);
                if (filefd < 0) {
                    string error_msg = "failed to open " + *commands[i].stdin_stream;
                    perror(error_msg.c_str());
                    exit(1);
                }
                dup2(filefd, STDIN_FILENO);
                close(filefd);
            } else if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }

            // stderr into file
            if (commands[i].stderr_stream) {
                int flags = (commands[i].stderr_stream->mode == FileMode::Overwrite) 
                    ? O_WRONLY | O_CREAT | O_TRUNC 
                    : O_WRONLY | O_CREAT | O_APPEND;
                int filefd = open(commands[i].stderr_stream->file.c_str(), flags, 0644);
                if (filefd < 0) {
                    string error_msg = "failed to open " + commands[i].stderr_stream->file;
                    perror(error_msg.c_str());
                    exit(1);
                }
                dup2(filefd, STDERR_FILENO);
                close(filefd);
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

            // getting environment - keep strings alive!
            vector<string> env_strings = get_env();
            vector<char*> envp;
            for (const string &s : env_strings) {
                envp.push_back(const_cast<char*>(s.c_str()));
            }
            envp.push_back(nullptr);

            // executing command
            execvpe(c_args[0], c_args.data(), envp.data());
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

vector<string> globber(const vector<Argument> &args) {
    vector<string> full_result;

    for (const Argument &arg : args) {
        if (!needs_glob(arg)) {
            full_result.push_back(arg.text);
            continue;
        }

        glob_t g{};
        int flags = GLOB_NOCHECK | GLOB_TILDE;

        int rc = glob(arg.text.c_str(), flags, nullptr, &g);
        if (rc == 0 || rc == GLOB_NOMATCH) {
            for (size_t i = 0; i < g.gl_pathc; ++i) {
                full_result.emplace_back(g.gl_pathv[i]);
            }
        } else {
            println("glob failed with code {} for pattern '{}'", rc, arg.text);
            // add original pattern if glob fails
            full_result.push_back(arg.text);
        }

        globfree(&g);
    }

    /*fmt::println("Globbed command:");
    for (const string &arg : full_result) {
        fmt::println("{}", arg);
    }*/

    return full_result;
}

void set_vars(vector<string> &command) {
    for (int i = 0; i < command.size(); i++) {
        vector<string> var_split = split_string(command[i], '=');
        if (var_split.size() == 2) {
            current_vars.push_back(command[i]);
            command.erase(command.begin() + i);
            i--;
        } else {
            break;
        }
    }
}

bool handle_command(BshContext &bsh_context) {
    vector<Argument> command_noglob = split_command(replace_env_vars(bsh_context.command));
    if (command_noglob.empty()) {
        return true;
    }
    
    // Handles ~, wildcards and stuff
    vector<string> command = globber(command_noglob);

    // sets variables for command
    current_vars.clear();
    set_vars(command);

    // set as env var
    if (command.empty()) {
        for (const string &var : current_vars) {
            vector<string> parameters = split_string(var, '=');
            set_env(parameters[0], parameters[1]);
        }
        return true;
    }

    string exe = command[0];
    vector<string> args(command.begin() + 1, command.end());
    bsh_context.args = args;
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
    } else if (exe == "export") {
        last_exit_code = export_command(bsh_context);
    } else if (exe == "unset") {
        last_exit_code = unset_command(bsh_context);
    } else {
        run_command(command, bsh_context.command);
    }

    return true;
}
