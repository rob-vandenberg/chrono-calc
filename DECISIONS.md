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

## Names and versions

- **D-003** Names: Chrono Calc (title), `chrono-calc` (identifier, repository, files), `chrono_calc` (C++ namespace), Calc (the application menu entry). (2026-10-30)
- **D-004** The project version lives only in the `project(... VERSION x.y.z)` line of the top-level `CMakeLists.txt`, starting at 0.0.1. The owner's `release` tool reads only `pyproject.toml`, so `gitpush.sh` commits and tags by itself. (2026-10-30)

## Packages

- **D-009** The `.deb` is built in a Debian 12 container and the `.rpm` in an AlmaLinux 9 container (the oldest supported systems; a C++ program is linked to the libraries of the system it is built on, and one built on the oldest runs on the newer ones), with podman. Each is tested in a bare Ubuntu 24.04 or Fedora 43 container as a normal user before the upload. The private `build-release.sh` does all of it; only the owner runs it, for milestone versions. (2026-10-30)

## GitHub

- **D-007** The repository `rob-vandenberg/chrono-calc` is private until the Standard view (M1) works; then the owner makes it public. (2026-10-30)
