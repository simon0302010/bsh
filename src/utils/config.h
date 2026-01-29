#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <toml++/toml.hpp>

#include "../core/context.h"

int load_config(const std::string &path);
std::string get_prompt(const BshContext &context);

#endif