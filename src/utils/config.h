#ifndef CONFIG_H
#define CONFIG_H

#include <string>

#include "../core/context.h"

int load_config(BshContext ctx);
std::string get_prompt(const BshContext &context);

#endif