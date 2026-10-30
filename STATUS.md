# STATUS.md

**Version:** 0.0.3
**Date:** 2026-10-30

## Where we are

Task 0.2 is done: Microsoft's engine is in `third_party/calculator/` and its 71 tests pass on Linux. Earlier: 0.0 set-up (0.0.1), 0.1 build skeleton (0.0.2).

- Git repository on `main`, pushed to GitHub (private).
- Build: `cmake --preset gcc` / `clang`, `cmake --build --preset gcc`, `ctest --preset gcc` (ARCHITECTURE.md 2). Warnings are errors; clang-tidy runs in every build of our code (D-012).
- Program: an empty window titled Chrono Calc (`src/app`, `src/ui`, `src/core`). Tests: `test_version`, `test_mainwindow`, `test_layers` (the layer check, D-011).
- Engine: `third_party/calculator/` (commit `79e1ac5`, unchanged, D-014), library `chrono_calc_engine`; Microsoft's 71 engine tests ported to Qt Test in `tests/engine/`, all passing with GCC, Clang and on Debian 12.
- Checked on the oldest target: Debian 12 (CMake 3.25.1, GCC 12.2, Qt 6.4.2) builds and passes the tests.
- Documents: `AGENTS.md`, `CLAUDE.md`, `CPP-DEVELOPMENT-RULES.md`, `ARCHITECTURE.md`, `ROADMAP.md`, `DECISIONS.md` (D-001 to D-014), `STATUS.md`, `DEVLOG.md`, `README.md`, `LICENSE` (AGPL-3.0-or-later).
- Private, git-ignored: `OWNER-RULES.md` (approved by the owner), `PROMPT-NEW-SESSION.md`, `gitpush.sh`, `build-release.sh` (packages in containers, D-009; works from task 9.2 on), `.claude/settings.local.json`, `scratchpad/`.
- `.claude/settings.local.json` (private) loads `OWNER-RULES.md` at every session start.
- GitHub: `rob-vandenberg/chrono-calc`, private until the first public release 1.0.z after M9 (D-007).
- Versions (D-013): 0.0.z for M0, 0.1.z for M1, 0.2.z for M2 and so on; the owner says when y changes.

The development plan (M0 to M10) is in `ROADMAP.md`, based on a survey of the engine (D-010). Reference clone of microsoft/calculator: `/home/rob/c++/calculator` (commit `79e1ac5`).

## Next task

**0.3 Wrapper and commands** (ROADMAP.md): connect the engine to `core` (`ICalcDisplay`), the command registry, the engine's texts in the Windows 7 wording. Needs a changelist and the owner's Proceed.

## Open questions

1. For task 0.3: Clang warns that the engine's `CCalcEngine::GetString(int)` (`CalcEngine.h:100`) can store a key that points to a temporary string when it looks up a text number that is not in its table. Latent: it only happens for an unknown text number. The engine is never edited; our wrapper must make sure every text number the engine asks for exists in the texts it is given.
