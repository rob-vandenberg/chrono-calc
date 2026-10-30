# STATUS.md

**Version:** 0.0.2
**Date:** 2026-10-30

## Where we are

Task 0.1 is done: the build skeleton. Task 0.0 (the project set-up) was pushed as 0.0.1.

- Git repository on `main`, pushed to GitHub (private).
- Build: `cmake --preset gcc` / `clang`, `cmake --build --preset gcc`, `ctest --preset gcc` (ARCHITECTURE.md 2). Warnings are errors; clang-tidy runs in every build of our code (D-012).
- Program: an empty window titled Chrono Calc (`src/app`, `src/ui`, `src/core`). Tests: `test_version`, `test_mainwindow`, `test_layers` (the layer check, D-011).
- Checked on the oldest target: Debian 12 (CMake 3.25.1, GCC 12.2, Qt 6.4.2) builds and passes the tests.
- Documents: `AGENTS.md`, `CLAUDE.md`, `CPP-DEVELOPMENT-RULES.md`, `ARCHITECTURE.md`, `ROADMAP.md`, `DECISIONS.md` (D-001 to D-012), `STATUS.md`, `DEVLOG.md`, `README.md`, `LICENSE` (AGPL-3.0-or-later).
- Private, git-ignored: `OWNER-RULES.md` (approved by the owner), `PROMPT-NEW-SESSION.md`, `gitpush.sh`, `build-release.sh` (packages in containers, D-009; works from task 9.2 on), `.claude/settings.local.json`, `scratchpad/`.
- `.claude/settings.local.json` (private) loads `OWNER-RULES.md` at every session start.
- GitHub: `rob-vandenberg/chrono-calc`, private until the first public release 1.0.z after M9 (D-007).
- Versions (D-013): 0.0.z for M0, 0.1.z for M1, 0.2.z for M2 and so on; the owner says when y changes.

The development plan (M0 to M10) is in `ROADMAP.md`, based on a survey of the engine (D-010). Reference clone of microsoft/calculator: `/home/rob/c++/calculator` (commit `79e1ac5`).

## Next task

**0.2 Engine import** (ROADMAP.md): copy the engine core from microsoft/calculator commit `79e1ac5` into `third_party/calculator/`, our own replacement for its `pch.h`, build with GCC and Clang, port Microsoft's engine tests. Needs a changelist and the owner's Proceed.

## Open questions

None.
