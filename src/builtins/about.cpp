#include "about.h"
#include <fmt/core.h>

using namespace fmt;

int show_about() {
    println("bsh Version 0.2");
    println("");
    println("bsh  Copyright (C) 2026  simon0302010");
    println("This program comes with ABSOLUTELY NO WARRANTY.");
    println("This is free software, and you are welcome to redistribute it");
    println("under certain conditions.");

    return 0;
}
