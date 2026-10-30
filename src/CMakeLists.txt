# --- Version ------------------------------------------------------------
# src/CMakeLists.txt 0.4.11

# --- Version history ----------------------------------------------------
# v0.4.11: core/statistics (the statistic functions), ui/statisticsparts (the list and its selection buttons)
# v0.3.10: ui/keyboardlayouts with XKB, xcb and Wayland (which layout a key comes from)
# v0.3.8: ui/scientificparts is ui/viewparts (choices, the bit display)
# v0.2.7: The Scientific view: core/layout (every view), core/view.h; ui/scientificparts
# v0.1.6: The Standard view: core has the layout, the menus and the colours; ui the window, the display, the buttons, the look
# v0.0.4: core holds the engine wrapper, the command registry and the typed keys; chrono-calc-cli
# v0.0.2: Initial: the layers core, ui and app (D-011)

# core: plain C++, no Qt (D-011)
add_library(chrono_calc_core STATIC core/version.cpp core/enginetexts.cpp core/commands.cpp core/calculator.cpp core/keys.cpp
    core/statistics.cpp
    core/menus.cpp core/layout.cpp)
target_include_directories(chrono_calc_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(chrono_calc_core PUBLIC chrono_calc_engine)
target_compile_definitions(chrono_calc_core PRIVATE CHRONO_CALC_VERSION="${PROJECT_VERSION}")
chrono_calc_strict(chrono_calc_core)

# ui: Qt Widgets, on top of core
add_library(chrono_calc_ui STATIC ui/mainwindow.cpp ui/mainwindow.h ui/calcbutton.cpp ui/calcbutton.h
    ui/displaypanel.cpp ui/displaypanel.h ui/look.cpp ui/look.h ui/fonts.cpp ui/fonts.h ui/viewparts.cpp ui/viewparts.h
    ui/keyboardlayouts.cpp ui/keyboardlayouts.h ui/statisticsparts.cpp ui/statisticsparts.h)
set_target_properties(chrono_calc_ui PROPERTIES AUTOMOC ON)
target_link_libraries(chrono_calc_ui PUBLIC chrono_calc_core Qt6::Widgets)
# The keyboard layouts of Linux (ui/keyboardlayouts): XKB, and the X11 and Wayland connections that give them
find_package(PkgConfig REQUIRED)
pkg_check_modules(CHRONO_CALC_KEYBOARD REQUIRED IMPORTED_TARGET xkbcommon xcb wayland-client)
target_link_libraries(chrono_calc_ui PRIVATE PkgConfig::CHRONO_CALC_KEYBOARD)
chrono_calc_strict(chrono_calc_ui)

# app: the program itself
add_executable(chrono-calc app/main.cpp)
target_link_libraries(chrono-calc PRIVATE chrono_calc_ui)
chrono_calc_strict(chrono-calc)

# chrono-calc-cli: the calculator without a screen, for tests and quick checks (task 0.4)
add_executable(chrono-calc-cli app/cli.cpp)
target_link_libraries(chrono-calc-cli PRIVATE chrono_calc_core)
chrono_calc_strict(chrono-calc-cli)
