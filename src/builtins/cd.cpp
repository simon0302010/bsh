#include <string>
#include <unistd.h>
#include <vector>
#include <filesystem>
#include <fmt/core.h>

#include "../core/context.h"

using namespace std;
using namespace fmt;

int pwd_command(BshContext &bsh_context) {
    fmt::println("{}", bsh_context.current_dir);
    return 0;
}

int cd_command(BshContext &bsh_context) {
    namespace fs = std::filesystem;

    fs::path target;

    if (bsh_context.args.empty()) {
        target = bsh_context.home_dir;
    } else {
        fs::path arg = bsh_context.args[0];
        target = arg.is_absolute()
            ? arg
            : fs::path(bsh_context.current_dir) / arg;
    }

    target = target.lexically_normal();

    std::error_code ec;
    fs::file_status st = fs::status(target, ec);

    if (ec) {
        println("cd: {}: {}", target.string(), ec.message());
        return ec.value();
    }

    if (!fs::exists(st)) {
        println("cd: no such file or directory: {}", target.string());
        return 2;
    }

    if (!fs::is_directory(st)) {
        println("cd: not a directory: {}", target.string());
        return 3;
    }

    if (::chdir(target.c_str()) != 0) {
        println("cd: permission denied: {}", target.string());
        return 4;
    }

    bsh_context.current_dir = fs::canonical(target).string();
    return chdir(bsh_context.current_dir.c_str());
}
