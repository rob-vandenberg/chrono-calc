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
