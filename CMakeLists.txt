cmake_minimum_required(VERSION 3.25)
project(chrono-calc VERSION 0.9.14 LANGUAGES CXX)

# --- Version ------------------------------------------------------------
# CMakeLists.txt 0.9.14

# --- Version history ----------------------------------------------------
# v0.9.14: M9 (versions 0.9.z): QtSvg for the icon; the install rules and CPack for the .deb and .rpm
# v0.8.13: M6, M7 and M8 (versions 0.8.z): Unit conversion, Date calculation, the Worksheets
# v0.5.12: M5 (versions 0.5.z): History, Digit grouping, Copy and Paste
# v0.4.11: M4 (versions 0.4.z): the Statistics view
# v0.3.10: The keyboard layouts in use (xkbcommon, xcb, wayland-client)
# v0.3.9: Letter keys on keyboards without Latin letters
# v0.3.8: M3 (versions 0.3.z): the Programmer view
# v0.2.7: M2 (versions 0.2.z): the Scientific view
# v0.1.6: The Standard view's window and the Windows 7 checks (tasks 1.1 to 1.5)
# v0.1.5: M1 (versions 0.1.z): the core of the Standard view
# v0.0.4: The project version for tasks 0.3 and 0.4
# v0.0.3: Microsoft's engine core in third_party/calculator (task 0.2)
# v0.0.2: The build skeleton (task 0.1): C++20, Qt 6 Widgets and Test, warnings as errors, clang-tidy, tests
# v0.0.1: Initial: the project version only; the build comes in task 0.1

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Svg draws the program's icon (packaging/chrono-calc.svg) in the window and the About box
find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Svg Test)

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

add_subdirectory(third_party/calculator)
add_subdirectory(src)

enable_testing()
add_subdirectory(tests)

# --- Installing and the packages (ROADMAP 9.2) ---------------------------
# The program, its "Calc" entry in the application menu, its icon, the app information for software centres, and the
# licences: ours, and the MIT licence of Microsoft's engine, which must go with every copy of it
include(GNUInstallDirs)
install(TARGETS chrono-calc RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES packaging/chrono-calc.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES packaging/chrono-calc.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(FILES packaging/io.github.rob_vandenberg.chrono_calc.metainfo.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES third_party/calculator/LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR}/third_party/calculator)

# The .deb and the .rpm (build-release.sh builds them in Debian 12 and AlmaLinux 9, D-009), named
# chrono-calc_<version>_amd64.deb and chrono-calc_<version>_x86_64.rpm; the libraries they need are found by the
# packaging tools from the program itself (dpkg-shlibdeps, rpm's automatic requirements)
set(CPACK_PACKAGE_NAME chrono-calc)
set(CPACK_PACKAGE_VENDOR "Rob Vandenberg")
set(CPACK_PACKAGE_CONTACT "Rob Vandenberg <robvandenberg64@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The calculator, with Standard, Scientific, Programmer and Statistics views")
set(CPACK_PACKAGE_DESCRIPTION "Chrono Calc is a free calculator with Standard, Scientific, Programmer and Statistics views, a history of calculations, unit conversion, date calculation and worksheets for mortgages, vehicle leases and fuel economy.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/rob-vandenberg/chrono-calc")
set(CPACK_PACKAGING_INSTALL_PREFIX /usr)
set(CPACK_STRIP_FILES ON)
set(CPACK_DEBIAN_PACKAGE_SECTION utils)
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# The platform plugins (X11, Wayland) are their own package on Debian and Ubuntu; without them the window cannot open
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-qpa-plugins")
set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_NAME}_${PROJECT_VERSION}_amd64.deb")
set(CPACK_RPM_PACKAGE_LICENSE "AGPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Productivity")
set(CPACK_RPM_PACKAGE_AUTOREQ ON)
set(CPACK_RPM_FILE_NAME "${CPACK_PACKAGE_NAME}_${PROJECT_VERSION}_x86_64.rpm")
# The folders every system already owns are not claimed by the package
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/applications /usr/share/icons /usr/share/icons/hicolor
    /usr/share/icons/hicolor/scalable /usr/share/icons/hicolor/scalable/apps /usr/share/metainfo /usr/share/doc)
include(CPack)
