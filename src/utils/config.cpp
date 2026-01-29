#include "core/context.h"
#include "utils/globals.h"
#include <iostream>
#include <fmt/base.h>
#include <filesystem>
#include <string>

#define TOML_EXCEPTIONS 0
#include <toml++/toml.hpp>

#include "utils.h"
#include "../core/context.h"

using namespace std;

static toml::table config;

int load_config(const string &path) {
    if (!filesystem::exists(path)) {
        return 2;
    }

    toml::parse_result result = toml::parse_file(path);

    if (result.failed()) {
        cout << result.error() << endl;
        return 1;
    } else {
        config = result.table();
        return 0;
    }
}

string get_prompt(const BshContext &context) {
    string prompt = config["prompt"].value_or("\033[34mUSERNAME@HOSTNAME:\033[36mWORKINGDIRECTORY\033[33mEXITCODE PROMPTSYMBOL ");
    // string prompt = "[DURATION] PROMPTSYMBOL ";

    replace_all(prompt, "USERNAME", context.username);
    replace_all(prompt, "HOSTNAME", context.hostname);
    replace_all(prompt, "WORKINGDIRECTORY", replace_all_return(context.current_dir, context.home_dir, "~"));
    replace_all(prompt, "EXITCODE", get_exit_code_string());
    replace_all(prompt, "PROMPTSYMBOL", get_prompt_symbol());
    replace_all(prompt, "DURATION", format_duration(last_command_duration));

    return prompt;
}