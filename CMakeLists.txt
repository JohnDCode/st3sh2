cmake_minimum_required(VERSION 3.10)
project(st3sh)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# Grab all .cpp files in src/
file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(st3sh ${SOURCES})

# Include headers directory
target_include_directories(st3sh PRIVATE src)
