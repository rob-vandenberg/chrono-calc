# DEVLOG.md

Append-only. One entry per completed task, newest last.

## 2026-10-30: 0.0.1, task 0.0 Project set up

- Chose C++20 with Qt 6 Widgets and CMake (D-001); the maths comes from the open-source engine of microsoft/calculator (D-002). Checked first: Microsoft never released the Windows 7 Calculator's own code, only the Windows 10 app (2019, MIT).
- Created the git repository, the project documents, the licence (AGPL-3.0-or-later, D-006), the version line in `CMakeLists.txt` (D-004), the private push script, the private release script `build-release.sh` (D-009), the owner-rules draft and the new-session hand-over.
- Created the private GitHub repository `rob-vandenberg/chrono-calc` (D-007).
- The owner approved `OWNER-RULES.md` (OR-1 to OR-15). Task 0.1 is next.

## 2026-10-30: 0.0.1, the development plan

- Cloned microsoft/calculator into `/home/rob/c++/calculator` (commit `79e1ac5`) as the reference and surveyed the engine: the core is portable C++, only `pch.h` and `UnitConverter` need Windows (D-010).
- Wrote the plan M0 to M10 into `ROADMAP.md`, with tasks 0.1 to 0.4 and 1.1 to 1.5 cut out.
- Open: the Windows 7 reference copy and the version-number scheme (STATUS.md).

## 2026-10-30: 0.0.2, task 0.1 Build skeleton

- CMake (C++20, Qt 6.4+ Widgets and Test), presets `gcc` and `clang` with Ninja, warnings as errors and clang-tidy on our own targets (D-012), tests with a 20-second limit and no screen.
- `src/core/version` (the project version), `src/ui/mainwindow` (an empty window titled Chrono Calc), `src/app/main.cpp`.
- Tests: `test_version`, `test_mainwindow`, `test_layers` (D-011). The layer check was proven on planted faults: a Qt include, a relative `../ui/` include, a lowercase `qstring.h`, a `.hpp` file, and a `../app/` include in ui; a legitimate relative include passes.
- clang-tidy's include check skips Qt's own headers; moc misread a regex literal, so the layer check reads include lines by hand.
- A fresh review found the layer-check gaps above (fixed) and that the release containers have no clang-tidy: `build-release.sh` 0.0.2 builds with it off.
- Built and tested in a Debian 12 container: CMake 3.25.1, GCC 12.2, Qt 6.4.2, no warnings, 3 of 3 tests pass. On this computer: GCC 15.2 and Clang 21.1, 3 of 3 on both.

## 2026-10-30: 0.0.3, task 0.2 Engine import

- Copied 53 files of microsoft/calculator `src/CalcManager` (commit `79e1ac5`) unchanged into `third_party/calculator/`, with `LICENSE` and `SOURCE.md`; left out `pch.h`, `UnitConverter`, `CalculatorVector.h` and the Windows build files (D-014).
- A trial compile showed only two needs: `pch.h`'s Windows headers (replaced by our `shim/pch.h`) and `<cmath>` for `Ratpack/support.cpp` (given by a compiler option). The engine builds with GCC 15, Clang 21 and on Debian 12 (GCC 12).
- Ported Microsoft's 71 engine tests to Qt Test (RationalTest 9, CalcInputTest 37, CalcEngineTests 4, CalculatorManagerTest 21) with the en-US engine texts (108); all pass with GCC, Clang and on Debian 12 (Qt 6.4). Two helpers ported them; a fresh reviewer compared every check and expected value with Microsoft's originals and found them faithful.
- Qt 6.4 has no `QTest::setThrowOnFail`: removed; a failed check in a helper still fails the test. clang-tidy's complexity check now ignores macros (every QCOMPARE hides a branch) instead of NOLINT blocks.
- The layer check also covers `third_party` (no Qt, core, ui or app), proven on two planted faults.
- Open for 0.3: the engine's `GetString(int)` can store a key to a temporary for an unknown text number (Clang warning); our wrapper must supply every text number.

## 2026-10-30: 0.0.4, tasks 0.3 Wrapper and commands, 0.4 Command-line driver

- `core/calculator` wraps the engine (Standard mode), `core/commands` is the command registry, `core/enginetexts` gives a text for every text number the engine can look up (closes the dangling-key path of `GetString(int)`; 108 en-US texts plus 41 of ours), `core/keys` maps typed characters (D-015).
- One memory as in Windows 7: MS replaces it. `chrono-calc-cli` runs key presses and commands without a screen (D-016).
- Windows 7 reference data collected through the Windows computer's Claude session: layout, menus, 20 results (`tests/reference/windows7-standard.md`). 19 of 20 numbers match; differences in percent after multiply/divide, in the expression line and in the decimal separator are for task 1.5.
- A fresh review found no bug and four weak tests (memory replace, CLI exit codes, CLI error text, the start value); all fixed: `memoryPlaces()`, `tests/cli_check.cmake`, the display starts empty until the engine reports.
- 11 tests pass with GCC 15, Clang 21 and on Debian 12.
