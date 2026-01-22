#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fmt/base.h>
#include <fmt/format.h>
#include <iterator>
#include <linux/limits.h>
#include <readline/rltypedefs.h>
#include <signal.h>
#include <stdexcept>
#include <string>
#include <fmt/core.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#include <readline/readline.h>
#include <vector>
#include <fstream>

#include "core/command.h"
#include "core/context.h"
#include "utils/utils.h"
#include "utils/globals.h"
#include "history/history.h"

using namespace std;

void sigint_handler(int s) {
    rl_free_line_state();
    rl_replace_line("", 0);
    rl_cleanup_after_signal();
    fmt::print("\n");
    rl_on_new_line();
    rl_redisplay();
    history_idx = -1;
}

string get_prompt_symbol() {
    string symbol = "$";
    if (geteuid() == 0) {
       symbol = "#";
    }

    if (last_exit_code != 0){
        return fmt::format("\033[1m\033[91m{}\033[0m", symbol);
    } else {
        return fmt::format("\033[1m\033[92m{}\033[0m", symbol);
    }
}

string read_file(const string &path) {
    ifstream file(path, ios::binary);
    if (!file) {
        fmt::println("bsh: file not found: {}", path);
        exit(2);
    }

    return string(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    );
}

int run_from_file(string file) {
    BshContext bsh_context;

    char cwd[PATH_MAX];
    if (strcmp(getcwd(cwd, PATH_MAX - 1), "Success") == 0) {
        perror("getcwd failed");
        return 1;
    } else {
        bsh_context.current_dir = string(cwd);
    }

    string file_contents = read_file(file);

    for (const string &command : prepare_input(file_contents)) {
        bsh_context.command = command;
        if (bsh_context.command.empty()) {
            continue;
        }
        if (!handle_command(bsh_context)) {
            return 0;
        }
    }

    return last_exit_code;
}

int main(int argc, char* argv[]) {
    // running script
    if (argc > 1) {
        return run_from_file(string(argv[1]));
    }

    // signal handler
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = sigint_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    // context struct
    BshContext bsh_context;

    // get home directory and username
    struct passwd *pw = getpwuid(getuid());

    string homedir = string(pw->pw_dir);
    string username = string(pw->pw_name);
    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, HOST_NAME_MAX - 1) != 0) {
        perror("gethostname failed");
    }
    char cwd[PATH_MAX];
    if (strcmp(getcwd(cwd, PATH_MAX - 1), "Success") == 0) {
        perror("getcwd failed");
        bsh_context.current_dir = homedir;
    } else {
        bsh_context.current_dir = string(cwd);
    }

    // main loop
    bsh_context.home_dir = homedir;

    history = read_history_file(homedir + "/.bsh_history");
    ofstream history_file(homedir + "/.bsh_history", ios::app);
    if (!history_file.is_open()) {
        perror("failed to open history file");
    }

    rl_bind_keyseq("\033[A", arrow_up);
    rl_bind_keyseq("\033[B", arrow_down);

    bool running = true;
    while (running) {
        string prompt = fmt::format("\033[34m{}@{}:\033[36m{}\033[33m{} {} ", username, hostname, replace_all(bsh_context.current_dir, homedir, "~"), get_exit_code_string(), get_prompt_symbol());
        const char* input_c = readline(prompt.c_str());
        if (input_c == nullptr) {
            break;
        }

        string input = string(input_c);
        for (const string &command : prepare_input(input)) {
            bsh_context.command = command;
            if (bsh_context.command.empty()) {
                continue;
            }
            history.push_back(command);
            history_file << command << endl;
            if (!handle_command(bsh_context)) {
                running = false;
            }
        }

        history_idx = -1;
        saved_line.clear();

        if (clear_history_file) {
            history.clear();
            history_file.close();
            ofstream ofs;
            ofs.open(homedir + "/.bsh_history", ofstream::out | ofstream::trunc);
            ofs.close();
            history_file.open(homedir + "/.bsh_history", ios::app);
            clear_history_file = false;
        }
    }

    if (history_file.is_open()) {
        history_file.close(); 
    }

    return 0;
}
