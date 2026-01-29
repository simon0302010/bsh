#include <iostream>
#include <fmt/base.h>
#include <filesystem>
#include <optional>

#define TOML_EXCEPTIONS 0
#include <toml++/toml.hpp>
#include <toml++/impl/forward_declarations.hpp>
#include <toml++/impl/parser.hpp>
#include <toml++/impl/table.hpp>

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

toml::table get_config() {
    if (config.empty()) {
        throw runtime_error("Config not loaded");
    }
    return config;
}