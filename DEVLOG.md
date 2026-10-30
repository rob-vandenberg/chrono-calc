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
