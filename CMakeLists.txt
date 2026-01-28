cmake_minimum_required(VERSION 3.14)
project(bsh VERSION 0.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

FetchContent_Declare(fmt
    URL https://github.com/fmtlib/fmt/releases/download/11.0.1/fmt-11.0.1.zip
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

file(GLOB SOURCES
    "src/*.cpp"
    "src/core/*.cpp"
    "src/builtins/*.cpp"
    "src/history/*.cpp"
    "src/utils/*.cpp"
)

add_executable(bsh ${SOURCES})
target_link_libraries(bsh PRIVATE
    fmt
    readline
)
target_include_directories(bsh PRIVATE src)

install(TARGETS bsh
    RUNTIME DESTINATION bin
)
