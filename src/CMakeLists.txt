# --- Version ------------------------------------------------------------
# src/CMakeLists.txt 0.0.4

# --- Version history ----------------------------------------------------
# v0.0.4: core holds the engine wrapper, the command registry and the typed keys; chrono-calc-cli
# v0.0.2: Initial: the layers core, ui and app (D-011)

# core: plain C++, no Qt (D-011)
add_library(chrono_calc_core STATIC core/version.cpp core/enginetexts.cpp core/commands.cpp core/calculator.cpp core/keys.cpp)
target_include_directories(chrono_calc_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(chrono_calc_core PUBLIC chrono_calc_engine)
target_compile_definitions(chrono_calc_core PRIVATE CHRONO_CALC_VERSION="${PROJECT_VERSION}")
chrono_calc_strict(chrono_calc_core)

# ui: Qt Widgets, on top of core
add_library(chrono_calc_ui STATIC ui/mainwindow.cpp ui/mainwindow.h)
set_target_properties(chrono_calc_ui PROPERTIES AUTOMOC ON)
target_link_libraries(chrono_calc_ui PUBLIC chrono_calc_core Qt6::Widgets)
chrono_calc_strict(chrono_calc_ui)

# app: the program itself
add_executable(chrono-calc app/main.cpp)
target_link_libraries(chrono-calc PRIVATE chrono_calc_ui)
chrono_calc_strict(chrono-calc)

# chrono-calc-cli: the calculator without a screen, for tests and quick checks (task 0.4)
add_executable(chrono-calc-cli app/cli.cpp)
target_link_libraries(chrono-calc-cli PRIVATE chrono_calc_core)
chrono_calc_strict(chrono-calc-cli)
