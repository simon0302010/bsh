#include <csignal>
#include <cstddef>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fmt/base.h>
#include <fmt/format.h>
#include <iterator>
#include <linux/limits.h>
#include <readline/rltypedefs.h>
#include <signal.h>
#include <string>
#include <fmt/core.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#include <readline/readline.h>
#include <vector>
#include <fstream>
#include <stdlib.h>

#include "core/command.h"
#include "core/context.h"
#include "utils/utils.h"
#include "utils/globals.h"
#include "history/history.h"
#include "utils/config.h"

using namespace std;

void sigint_handler(int s) {
    rl_free_line_state();
    rl_replace_line("", 0);
    rl_cleanup_after_signal();
    fmt::print("\n");
    rl_on_new_line();
    rl_redisplay();
    history_idx = -1;
    last_command_duration = 0;
    chrono::system_clock::time_point now = chrono::system_clock::now();
    last_command_timestamp = static_cast<long>(chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count());
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

char **filename_completition(const char *text, int start, int end) {
    rl_filename_quoting_desired = 1;
    return 0;
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

    // loading config
    // auto config = 

    // main loop
    bsh_context.home_dir = homedir;
    bsh_context.username = username;
    bsh_context.hostname = hostname;

    history = read_history_file(homedir + "/.bsh_history");
    ofstream history_file(homedir + "/.bsh_history", ios::app);
    if (!history_file.is_open()) {
        perror("failed to open history file");
    }

    rl_bind_keyseq("\033[A", arrow_up);
    rl_bind_keyseq("\033[B", arrow_down);
    // Might add back when better
    // rl_startup_hook = startup_hook;
    // rl_event_hook = check_buffer; 

    rl_attempted_completion_function = filename_completition;
    rl_completer_quote_characters = "\"";
    rl_filename_quote_characters  = " ";

    bool running = true;
    while (running) {
        string prompt = get_prompt(bsh_context);
        char* input_c = readline(prompt.c_str());
        if (input_c == nullptr) {
            break;
        }

        last_command_duration = 0;
        chrono::system_clock::time_point now = chrono::system_clock::now();
        last_command_timestamp = static_cast<long>(chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count());

        string input = string(input_c);
        ::free(input_c);
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
