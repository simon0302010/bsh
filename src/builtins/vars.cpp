#include <string>
#include <vector>

#include "../utils/utils.h"
#include "../utils/globals.h"
#include "core/context.h"
#include "fmt/base.h"
#include "fmt/core.h"

using namespace std;
using namespace fmt;

int export_command(const BshContext &context) {
    if (context.args.empty()) {
        for (const string &var : get_env()) {
            println("{}", var);
        }
    } else {
        for (const string &arg : context.args) {
            vector<string> parameters = split_string(arg, '=');
            if (parameters.size() != 2) {
                println("export: invalid syntax");
                return 1;
            } else {
                set_env(parameters[0], parameters[1]);
            }
        }
    }

    return 0;
}

int unset_command(const BshContext &context) {
    if (context.args.empty()) {
        println("unset: not enough arguments");
        return 1;
    } else {
        for (const string &arg : context.args) {
            environment_vars.erase(arg);
            unset_vars.push_back(arg);
        }
    }

    return 0;
}