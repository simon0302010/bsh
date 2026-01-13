#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ios>
#include <iostream>
#include <signal.h>
#include <string>

using namespace std;

void handle_command(string command) {
    if (command == "exit") {
        exit(0);
    }
}

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

    // main loop
    string command;
    while (true) {
        printf("$ ");
        if (!getline(cin, command)) {
            cin.clear();
            continue;
        }
        handle_command(command);
    }
}
