# ROADMAP.md

Status per task: `todo`, `doing`, `done`. Every task is its own changelist and Proceed; the owner tests every pushed version.

The engine survey behind this plan (2026-10-30, microsoft/calculator commit `79e1ac5`, reference clone in `/home/rob/c++/calculator`): the engine core (`CEngine`, `Ratpack`, `CalculatorManager`, `CalculatorHistory`) is portable C++; only its `pch.h` includes Windows files. `UnitConverter` needs the Windows-only PPL tasks and its unit lists are Windows 10 lists in C#. Windows 10 has no Statistics view and no Worksheets, and its Date calculation is C#. Microsoft's engine tests are C++ (`CalcEngineTests`, `CalcInputTest`, `CalculatorManagerTest`, `RationalTest`). See D-010.

## M0: Foundation (no window yet)

| Task | What | Status |
|---|---|---|
| 0.0 | Project set up: git, documents, rules, push and release scripts, private GitHub repository | done |
| 0.1 | Tools and skeleton: list what to install (Qt 6, CMake, clang-tidy); CMake with Qt 6 Widgets and Qt Test, warnings as errors, `.clang-tidy`, an empty window titled Chrono Calc, a first test, the layer check | todo |
| 0.2 | Engine import: copy the engine core from commit `79e1ac5` into `third_party/calculator/` with `LICENSE` and `SOURCE.md`, our own replacement for `pch.h` in `core`; build with GCC and Clang; port Microsoft's engine tests to Qt Test | todo |
| 0.3 | Wrapper and commands: connect the engine to `core` (`ICalcDisplay`), the command registry, the engine's texts (error messages) in the Windows 7 wording | todo |
| 0.4 | A command-line test driver: runs any key sequence without a screen, for tests and quick result checks | todo |

## M1: Standard view (the first usable version; then the repository goes public, D-007)

| Task | What | Status |
|---|---|---|
| 1.1 | Window and look: menus (View, Edit, Help), the display (expression line, value, the memory "M"), `colours.h`, our own button drawing close to Windows 7 | todo |
| 1.2 | Buttons and keys: the Standard grid and every keyboard key | todo |
| 1.3 | Memory: MC, MR, MS, M+, M− | todo |
| 1.4 | Window behaviour: fixed size per view, the position remembered | todo |
| 1.5 | Reference check: a table of key presses with the results the real Windows 7 Calculator shows, run as tests | todo |

## M2: Scientific view

| Task | What | Status |
|---|---|---|
| 2.1 | All Scientific buttons and keys; Degrees, Radians, Grads; Inv; F-E, dms (the engine has the functions) | todo |

## M3: Programmer view

| Task | What | Status |
|---|---|---|
| 3.1 | Hex, Dec, Oct, Bin; Qword, Dword, Word, Byte; bit operations (the engine has them); the bit display (ours) | todo |

## M4: Statistics view

| Task | What | Status |
|---|---|---|
| 4.1 | The value list, Count, the statistic functions: our own code on the engine's exact fractions (D-010) | todo |

## M5: History, Digit grouping, Edit

| Task | What | Status |
|---|---|---|
| 5.1 | History (Ctrl+H; the engine keeps it, the list and editing are ours), Digit grouping, Copy and Paste, the remembered settings | todo |

## M6: Unit conversion

| Task | What | Status |
|---|---|---|
| 6.1 | The side panel with every unit type and unit of Windows 7; our own code (D-010), factors checked against Microsoft's data | todo |

## M7: Date calculation

| Task | What | Status |
|---|---|---|
| 7.1 | The difference between two dates; add or subtract days; our own code (D-010) | todo |

## M8: Worksheets

| Task | What | Status |
|---|---|---|
| 8.1 | Mortgage, Vehicle lease, Fuel economy (mpg), Fuel economy (L/100 km); our own code on the engine's exact fractions (D-010), checked against Windows 7 | todo |

## M9: Release

| Task | What | Status |
|---|---|---|
| 9.1 | Help and About; our own icon; the Calc menu entry and AppStream file | todo |
| 9.2 | `.deb` and `.rpm` packages: the CPack settings (`include(CPack)`, package names, dependencies) that `build-release.sh` needs; tested on Debian/Ubuntu and Fedora/RHEL, on KDE and GNOME | todo |

## M10: Windows (optional, the owner decides)

| Task | What | Status |
|---|---|---|
| 10.1 | A Windows build and installer | todo |
