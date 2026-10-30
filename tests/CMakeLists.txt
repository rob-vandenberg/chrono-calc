# --- Version ------------------------------------------------------------
# tests/CMakeLists.txt 0.0.4

# --- Version history ----------------------------------------------------
# v0.0.4: test_calculator (the engine wrapper, commands and keys); chrono-calc-cli checked on exit code, output and error
# v0.0.3: Microsoft's engine tests in engine/; the layer check also reads third_party/
# v0.0.2: Initial: the version, main window and layer tests; 20 seconds at most each, no screen needed

# One Qt Test program per file; each test runs without a screen and stops after 20 seconds (AGENTS.md 4.5.3)
function(chrono_calc_test name)
    add_executable(${name} ${name}.cpp)
    set_target_properties(${name} PROPERTIES AUTOMOC ON)
    target_link_libraries(${name} PRIVATE Qt6::Test ${ARGN})
    chrono_calc_strict(${name})
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES TIMEOUT 20 ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

chrono_calc_test(test_version chrono_calc_core)
target_compile_definitions(test_version PRIVATE CHRONO_CALC_PROJECT_VERSION="${PROJECT_VERSION}")

chrono_calc_test(test_mainwindow chrono_calc_ui)

chrono_calc_test(test_calculator chrono_calc_core)

# The command-line driver, checked on exit code, output and error text (cli_check.cmake)
function(chrono_calc_cli_test name exit_code output)
    add_test(NAME ${name} COMMAND ${CMAKE_COMMAND} -DCLI=$<TARGET_FILE:chrono-calc-cli> "-DARGUMENTS=${ARGN}"
        -DEXIT=${exit_code} "-DOUTPUT=${output}" ${CHRONO_CALC_CLI_ERROR} -P ${CMAKE_CURRENT_SOURCE_DIR}/cli_check.cmake)
    set_tests_properties(${name} PROPERTIES TIMEOUT 20)
endfunction()

# 12 + 3 x 2 = 30 from left to right, then a command id, then the memory
chrono_calc_cli_test(cli_runs_keys_and_commands 0 "display: 0\nexpression: \nerror: no\nmemory: yes\n" 12+3*2= memory.store clear)
chrono_calc_cli_test(cli_shows_the_result 0 "^display: 30\n" 12+3*2=)
set(CHRONO_CALC_CLI_ERROR "-DERROR=cannot read 'x' in 2x3")
chrono_calc_cli_test(cli_refuses_an_unknown_argument 2 "^$" 2x3)
unset(CHRONO_CALC_CLI_ERROR)

chrono_calc_test(test_layers)
target_compile_definitions(test_layers PRIVATE CHRONO_CALC_SOURCE_DIR="${PROJECT_SOURCE_DIR}/src"
    CHRONO_CALC_THIRD_PARTY_DIR="${PROJECT_SOURCE_DIR}/third_party")

add_subdirectory(engine)
