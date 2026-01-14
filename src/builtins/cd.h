#ifndef CD_COMMAND_H
#define CD_COMMAND_H

#include <string>
#include <vector>

#include "../core/context.h"

void cd_command(BshContext &bsh_context, const std::vector<std::string> &args);

#endif
