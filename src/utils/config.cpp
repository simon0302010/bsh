#include <optional>
#include <fmt/base.h>
#include <filesystem>

#define TOML_EXCEPTIONS 0
#include <toml++/toml.hpp>
#include <toml++/impl/forward_declarations.hpp>
#include <toml++/impl/parser.hpp>

using namespace std;

static toml::parse_result config;

int load_config(const string &path) {
    if (!filesystem::exists(path)) {
        return 2;
    }

    config = toml::parse_file(path);

    if (!config) {
        fmt::println("{}", config.error())
    }
}