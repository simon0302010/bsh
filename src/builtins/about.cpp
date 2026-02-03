#include "about.h"
#include "core/context.h"
#include <fmt/core.h>

using namespace fmt;

int about_command(BshContext &) {
    println("bsh Version 0.3");
    println("");
    println("bsh  Copyright (C) 2026  simon0302010");
    println("This program comes with ABSOLUTELY NO WARRANTY.");
    println("This is free software, and you are welcome to redistribute it");
    println("under certain conditions.");

    return 0;
}
