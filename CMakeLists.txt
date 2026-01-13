cmake_minimum_required(VERSION 3.10)
project(bsh VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES "src/*.cpp")

add_executable(bsh ${SOURCES})
target_link_libraries(bsh PRIVATE
    fmt
    readline
)
target_include_directories(bsh PRIVATE src)
