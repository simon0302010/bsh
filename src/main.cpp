#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <iostream>
#include <signal.h>
#include <string>
#include <fmt/base.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>

#include "command.h"
#include "structs.h"
#include "utils.h"

using namespace std;

void sigint_handler(int s) {
    printf("\n");
}

int main() {
    // signal handler
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = sigint_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    // get home directory and username
    struct passwd *pw = getpwuid(getuid());

    string homedir = string(pw->pw_dir);
    string username = string(pw->pw_name);
    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, HOST_NAME_MAX - 1) != 0) {
        perror("gethostname failed");
    }

    // main loop
    BshContext bsh_context;
    bsh_context.current_dir = homedir;
    bsh_context.home_dir = homedir;

    while (true) {
        fmt::print("{}@{}:{} $ ", username, hostname, replace_all(bsh_context.current_dir, homedir, "~"));
        if (!getline(cin, bsh_context.command)) {
            cin.clear();
            continue;
        }
        handle_command(bsh_context);
    }
}
