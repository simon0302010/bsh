#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fmt/base.h>
#include <linux/limits.h>
#include <signal.h>
#include <string>
#include <fmt/core.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "command.h"
#include "structs.h"
#include "utils.h"

using namespace std;

void sigint_handler(int s) {
    rl_free_line_state();
    rl_cleanup_after_signal();
    fmt::print("\n");
    rl_on_new_line();
    rl_redisplay();
}

int main() {
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

    while (true) {
        string prompt = fmt::format("{}@{}:{} $ ", username, hostname, replace_all(bsh_context.current_dir, homedir, "~"));
        const char* command = readline(prompt.c_str());
        bsh_context.command = string(command);
        if (bsh_context.command.empty()) {
            continue;
        }
        add_history(command);
        handle_command(bsh_context);
    }
}
