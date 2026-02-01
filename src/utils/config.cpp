#include "core/context.h"
#include "utils/globals.h"
#include <fmt/base.h>
#include <string>
#include <fstream>

#include "utils.h"
#include "../core/context.h"
#include "../core/command.h"

using namespace std;

int load_config(BshContext ctx) {
    ifstream file(ctx.config_path, ios::binary);
    if (!file) {
        return 2;
    }

    string file_contents = string(
        (istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>()
    );

    for (const string &command : prepare_input(file_contents)) {
        ctx.command = command;
        if (ctx.command.empty()) {
            continue;
        }
        if (!handle_command(ctx)) {
            exit(0);
        }
        if (last_exit_code != 0) {
            fmt::println("warning: \"{}\" exited with code {}", ctx.command, last_exit_code);
        }
    }

    last_exit_code = 0;
    last_command_duration = 0;

    return 0;
}

string get_prompt(const BshContext &context) {
    string prompt = get_var_or("PROMPT", "\033[34mUSERNAME@HOSTNAME:\033[36mWORKINGDIRECTORY\033[33mEXITCODE PROMPTSYMBOL ");

    replace_all(prompt, "USERNAME", context.username);
    replace_all(prompt, "HOSTNAME", context.hostname);
    replace_all(prompt, "WORKINGDIRECTORY", replace_all_return(context.current_dir, context.home_dir, "~"));
    replace_all(prompt, "EXITCODE", get_exit_code_string());
    replace_all(prompt, "PROMPTSYMBOL", get_prompt_symbol());
    replace_all(prompt, "DURATION", format_duration(last_command_duration));
    replace_all(prompt, "TIME", get_time_formatted());

    return prompt;
}