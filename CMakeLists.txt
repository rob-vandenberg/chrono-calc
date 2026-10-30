cmake_minimum_required(VERSION 3.25)
project(chrono-calc VERSION 0.0.2 LANGUAGES CXX)

# --- Version ------------------------------------------------------------
# CMakeLists.txt 0.0.2

# --- Version history ----------------------------------------------------
# v0.0.2: The build skeleton (task 0.1): C++20, Qt 6 Widgets and Test, warnings as errors, clang-tidy, tests
# v0.0.1: Initial: the project version only; the build comes in task 0.1

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Test)

# clang-tidy is the hard gate (AGENTS.md 3): on by default, for our own targets only
option(CHRONO_CALC_CLANG_TIDY "Run clang-tidy on our own code during the build" ON)
if(CHRONO_CALC_CLANG_TIDY)
    find_program(CHRONO_CALC_CLANG_TIDY_PROGRAM NAMES clang-tidy REQUIRED)
endif()

# Our own targets: warnings are errors, and clang-tidy runs on them (third_party/ never gets this)
function(chrono_calc_strict target)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    if(CHRONO_CALC_CLANG_TIDY)
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${CHRONO_CALC_CLANG_TIDY_PROGRAM}")
    endif()
endfunction()

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
