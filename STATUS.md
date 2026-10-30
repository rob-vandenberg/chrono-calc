# STATUS.md

**Version:** 0.0.4
**Date:** 2026-10-30

## Where we are

M0 is done. Tasks 0.3 (engine wrapper, command registry, engine texts) and 0.4 (`chrono-calc-cli`) landed in 0.0.4. Earlier: 0.0 set-up (0.0.1), 0.1 build skeleton (0.0.2), 0.2 engine import (0.0.3). The owner told Claude to continue on its own through M1 (versions 0.1.z).

- Git repository on `main`, pushed to GitHub (private).
- Build: `cmake --preset gcc` / `clang`, `cmake --build --preset gcc`, `ctest --preset gcc` (ARCHITECTURE.md 2). Warnings are errors; clang-tidy runs in every build of our code (D-012).
- Program: an empty window titled Chrono Calc (`src/app`, `src/ui`, `src/core`). Tests: `test_version`, `test_mainwindow`, `test_layers` (the layer check, D-011).
- Engine: `third_party/calculator/` (commit `79e1ac5`, unchanged, D-014), library `chrono_calc_engine`; Microsoft's 71 engine tests ported to Qt Test in `tests/engine/`, all passing with GCC, Clang and on Debian 12.
- Core: `core/calculator` (the engine wrapper, D-015), `core/commands`, `core/enginetexts`, `core/keys`; `chrono-calc-cli` (D-016) runs key presses and commands without a screen.
- Windows 7 reference data for the Standard view (layout, menus, 20 results), measured on the owner's Windows computer: `tests/reference/windows7-standard.md`.
- Checked on the oldest target: Debian 12 (CMake 3.25.1, GCC 12.2, Qt 6.4.2) builds and passes the tests.
- Documents: `AGENTS.md`, `CLAUDE.md`, `CPP-DEVELOPMENT-RULES.md`, `ARCHITECTURE.md`, `ROADMAP.md`, `DECISIONS.md` (D-001 to D-014), `STATUS.md`, `DEVLOG.md`, `README.md`, `LICENSE` (AGPL-3.0-or-later).
- Private, git-ignored: `OWNER-RULES.md` (approved by the owner), `PROMPT-NEW-SESSION.md`, `gitpush.sh`, `build-release.sh` (packages in containers, D-009; works from task 9.2 on), `.claude/settings.local.json`, `scratchpad/`.
- `.claude/settings.local.json` (private) loads `OWNER-RULES.md` at every session start.
- GitHub: `rob-vandenberg/chrono-calc`, private until the first public release 1.0.z after M9 (D-007).
- Versions (D-013): 0.0.z for M0, 0.1.z for M1, 0.2.z for M2 and so on; the owner says when y changes.

The development plan (M0 to M10) is in `ROADMAP.md`, based on a survey of the engine (D-010). Reference clone of microsoft/calculator: `/home/rob/c++/calculator` (commit `79e1ac5`).

## Next task

**1.1 Window and look** (ROADMAP.md), then 1.2 to 1.5. Differences from Windows 7 found so far, for task 1.5: percent after multiply or divide (Windows 7: 100*5% = 5; the Windows 10 engine gives 0.05); the expression line (Windows 7 keeps the whole chain `2 + 3 *`, uses `*` and `/`, shows `sqrt(9)` and `reciproc(5)`, and is empty after =); the decimal separator follows the regional settings.

## Open questions

None.
