# DECISIONS.md

Every design decision as it stands now, by topic, short. Every decision here was approved through the owner's Proceed. New decisions get the next free number.

## The product

- **D-008** Chrono Calc does everything the Windows 7 Calculator does: the same results, views, side panels, keys, shortcuts, menus and layout (AGENTS.md §0). (2026-10-30)
- **D-005** Our own look, very very close to the old calculator: same layout, proportions, colours and feel; every image, icon and button drawing is our own; nothing of Microsoft's artwork is copied. (2026-10-30)

## Technology

- **D-001** C++20 with Qt 6 Widgets, built with CMake. Reason: the open-source engine of the Windows calculator is C++ and can be used as it is; Qt is native on KDE and works on GNOME and Windows; one small program, quick to start, simple to package. Python with PySide6 was rejected (rewrite of the engine or two languages, slower start, heavier packages). (2026-10-30)
- **D-006** The licence is AGPL-3.0-or-later, the same as Chrono Mail. The MIT engine is compatible. (2026-10-30)

## The engine

- **D-002** The maths is the engine of microsoft/calculator (`CalcManager`, `RatPack`, MIT, released 2019), copied unchanged into `third_party/calculator/`, from commit `79e1ac5` (2026-10-20); a reference clone of the whole repository is in `/home/rob/c++/calculator`. Microsoft never released the Windows 7 Calculator's own code; the Windows 10 engine descends from it. (2026-10-30)

- **D-010** Only the engine core is imported: `CEngine`, `Ratpack`, `CalculatorManager`, `CalculatorHistory` and their headers; its `pch.h` is replaced by our own in `core`. `UnitConverter` is left out (it needs the Windows-only PPL tasks, and its unit lists are Windows 10's). Unit conversion, Date calculation, the Statistics view and the Worksheets are our own code in `core`, because Windows 10 has none of them in C++; every result they give is computed with the engine's exact fractions (`Rational`), never with floating-point maths. Microsoft's C++ engine tests are ported to Qt Test to prove the engine works the same on Linux. (2026-10-30)

## Build and checks

- **D-011** The layers are `third_party` <- `core` <- `ui` <- `app`: a layer includes only from the layers below it; `core` is plain C++ without Qt. `tests/test_layers.cpp` enforces it (OR-13) by reading every `#include` in `src/`. (2026-10-30)
- **D-012** Two builds, GCC and Clang (`CMakePresets.json`: `gcc`, `clang`, Ninja, `build/gcc`, `build/clang`). clang-tidy runs during every build of our own code with the families bugprone, clang-analyzer, cppcoreguidelines, modernize, performance, readability and misc, every finding an error; off: trailing return types, magic numbers, identifier length, owning-memory (Qt parents own their children); `misc-include-cleaner` skips Qt's own headers. Each reason is in `.clang-tidy`. clang-tidy is the developer's gate on this computer; the release builds in containers (D-009) run without it (`-DCHRONO_CALC_CLANG_TIDY=OFF`). (2026-10-30)

- **D-014** The engine import (task 0.2), described in `third_party/calculator/SOURCE.md`: 53 files of `src/CalcManager` copied unchanged, with Microsoft's `LICENSE`; left out are `pch.h` (Windows-only headers), `UnitConverter` (D-010), `CalculatorVector.h` (unused) and the Windows build files. Two measures of ours, both outside Microsoft's files: `third_party/calculator/shim/pch.h` (the same standard headers without the Windows ones; it lives in `third_party`, not `core`, because a lower layer may not include a higher one, D-011), and `<cmath>` given to `Ratpack/support.cpp` through a compiler option. The engine is the library `chrono_calc_engine`, built without warnings-as-errors and without clang-tidy; its headers are system includes for our code. Microsoft's engine tests (71) are ported to Qt Test in `tests/engine/`, every check and expected value unchanged, with the en-US engine texts of `CEngineStrings.resw` in `engine_resource_provider.cpp`. The layer check also covers `third_party`: no Qt, `core`, `ui` or `app`. (2026-10-30)

- **D-015** The engine wrapper (task 0.3): `core/calculator` holds one `CalculatorManager` in Standard mode and keeps what it reports (number, expression line, error, memory) as plain values; the expression line is the engine's tokens joined as they come (they carry their own spacing). Commands go in by id through the registry `core/commands` (`digit.0`-`digit.9`, `decimal`, `add`, `subtract`, `multiply`, `divide`, `equals`, `clear`, `clear_entry`, `backspace`, `negate`, `sqrt`, `percent`, `reciprocal`, `memory.clear/recall/store/add/subtract`, `view.standard`). One memory, as in Windows 7: MS replaces it (the engine would add a new memory place), MR only when a number is in memory. `core/enginetexts` gives a text for every text number the engine can look up, because the engine's `GetString(int)` stores a key pointing to a temporary string for a number without a text: the 108 en-US texts of `CEngineStrings.resw` plus 41 of ours (digits, memory names, five rare error messages), to be checked against Windows 7 in task 1.5. The number separators are en-US until Digit grouping (M5). (2026-10-30)
- **D-016** `chrono-calc-cli` (task 0.4): the calculator without a screen. Each argument is a command id or a run of typed characters (`core/keys`: digits, `.` `,` `+` `-` `*` `/` `=` Enter `@` `%` `r`, as in Windows 7); it prints the display, the expression line, error and memory. Used by tests and for quick checks. (2026-10-30)

## Names and versions

- **D-003** Names: Chrono Calc (title), `chrono-calc` (identifier, repository, files), `chrono_calc` (C++ namespace), Calc (the application menu entry). (2026-10-30)
- **D-004** The project version lives only in the `project(... VERSION x.y.z)` line of the top-level `CMakeLists.txt`, starting at 0.0.1. The owner's `release` tool reads only `pyproject.toml`, so `gitpush.sh` commits and tags by itself. (2026-10-30)

## Packages

- **D-009** The `.deb` is built in a Debian 12 container and the `.rpm` in an AlmaLinux 9 container (the oldest supported systems; a C++ program is linked to the libraries of the system it is built on, and one built on the oldest runs on the newer ones), with podman. Each is tested in a bare Ubuntu 24.04 or Fedora 43 container as a normal user before the upload. The private `build-release.sh` does all of it; only the owner runs it, for milestone versions. (2026-10-30)

- **D-013** Version numbers (the owner, 2026-10-30): the foundation (M0) is 0.0.z; M1 is 0.1.z, M2 0.2.z, M3 0.3.z and so on; the first public release is 1.0.z, when M9 is finished. Only the owner changes x and y and says when; z keeps counting and never resets (0.0.5 is followed by 0.1.6). (2026-10-30)

## GitHub

- **D-007** The repository `rob-vandenberg/chrono-calc` stays private through M8; the owner makes it public with the first public release, 1.0.z, when M9 is finished (D-013). (2026-10-30)
