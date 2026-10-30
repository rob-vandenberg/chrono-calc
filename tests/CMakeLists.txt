# --- Version ------------------------------------------------------------
# tests/CMakeLists.txt 0.0.3

# --- Version history ----------------------------------------------------
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

chrono_calc_test(test_layers)
target_compile_definitions(test_layers PRIVATE CHRONO_CALC_SOURCE_DIR="${PROJECT_SOURCE_DIR}/src"
    CHRONO_CALC_THIRD_PARTY_DIR="${PROJECT_SOURCE_DIR}/third_party")

add_subdirectory(engine)
