# STATUS.md

**Version:** 0.0.1 (not pushed yet; the first push makes the first commit)
**Date:** 2026-10-30

## Where we are

Task 0.0 is done: the project is set up.

- Git repository on `main`, no commit yet.
- Documents: `AGENTS.md`, `CLAUDE.md`, `CPP-DEVELOPMENT-RULES.md`, `ARCHITECTURE.md`, `ROADMAP.md`, `DECISIONS.md` (D-001 to D-010), `STATUS.md`, `DEVLOG.md`, `README.md`, `LICENSE` (AGPL-3.0-or-later).
- `CMakeLists.txt` holds only the project version (0.0.1); the build comes in task 0.1.
- Private, git-ignored: `OWNER-RULES.md` (approved by the owner), `PROMPT-NEW-SESSION.md`, `gitpush.sh`, `build-release.sh` (packages in containers, D-009; works from task 9.2 on), `.claude/settings.local.json`, `scratchpad/`.
- `.claude/settings.local.json` (private) loads `OWNER-RULES.md` at every session start.
- GitHub: `rob-vandenberg/chrono-calc`, private (D-007), empty until the first push.

The development plan (M0 to M10) is in `ROADMAP.md`, based on a survey of the engine (D-010). Reference clone of microsoft/calculator: `/home/rob/c++/calculator` (commit `79e1ac5`).

## Next task

**0.1 Build skeleton** (ROADMAP.md): CMake with Qt 6 Widgets and Qt Test, warnings as errors, `.clang-tidy`, an empty window titled Chrono Calc, one test, the layer check. Needs a changelist and the owner's Proceed.

## Open questions

1. The Windows 7 reference: exact results, unit lists, worksheet answers and texts can only come from a running Windows 7 Calculator; needed from task 1.5 on. Does the owner have one, and on which computer?
2. Version numbers: proposed that y goes up by one per finished milestone (0.1.z after M1) and 1.0 comes after M9; the owner decides x and y.
3. Not yet checked: which Qt 6, CMake, compiler and clang-tidy versions are installed on this computer (task 0.1 checks and lists what to install).
