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
    string args = context.command.substr(6);
    vector<string> parameters = split_string(args, '=');

    if (parameters.size() != 2) {
        println("Usage: export KEY=VALUE");
        return 1;
    }

    string key = parameters[0];
    ltrim(key);
    rtrim(key);

    string value = parameters[1];
    ltrim(value);
    rtrim(value);

    environment_vars[key] = value;

    return 0;
}