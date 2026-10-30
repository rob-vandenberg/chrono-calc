# AGENTS.md: guide for AI agents and contributors

Chrono Calc is an open-source desktop calculator for Linux (KDE first, GNOME as well; Debian and Red Hat families; Windows possibly later), written in C++ with a Qt 6 Widgets user interface. It does everything the Windows 7 Calculator does, in a look of its own that stays very close to it.

## 0. The basic rule (D-008, D-005; only 4.1 Never a wrong result comes before it)

Chrono Calc behaves like the **Windows 7 Calculator**:

1. The same results: the maths is done by the calculator engine Microsoft published as open source (microsoft/calculator, MIT), unchanged.
2. The same seven views: Standard, Scientific, Programmer, Statistics, and the side panels Unit conversion, Date calculation and Worksheets (Mortgage, Vehicle lease, Fuel economy mpg and L/100 km); plus History and Digit grouping.
3. The same keys, keyboard shortcuts and menus (View, Edit, Help), and the same layout and window sizes.
4. **Our own look, very very close to the old calculator** (D-005): same layout, proportions, colours and feel, but every image, icon and button drawing is our own. Nothing of Microsoft's artwork is copied, and the names "Windows" and "Microsoft" appear only where the engine's licence requires them.

Every changelist says how it keeps this rule. What the rule does not settle is asked, never chosen.

## 1. Orientation

| File | Purpose | When to read |
|---|---|---|
| `AGENTS.md` | Project-specific technical rules (this file) | Every session, first |
| `CPP-DEVELOPMENT-RULES.md` | How we work together: the owner's working rules for C++ projects | Every session, second |
| `OWNER-RULES.md` | Every rule the owner set, numbered (OR-nn); only the owner changes it; private: on this computer only, not in git | Every session, third (also loaded by itself) |
| `STATUS.md` | Where we are now, the next task, open questions | Every session, fourth |
| `DECISIONS.md` | Every design decision as it stands now, by topic, short | Every session, after STATUS.md; before proposing to change a design choice |
| `ARCHITECTURE.md` | Folders, layers, how the engine is wrapped | Before any code task |
| `ROADMAP.md` | Milestones and tasks, each with a status | When picking or finishing a task |
| `DEVLOG.md` | Append-only log, one entry per completed task | When you need history |
| `PROMPT-NEW-SESSION.md` | The owner's hand-over prompt for a new chat; private | When the owner gives it |

## 2. Naming

| Context | Form |
|---|---|
| Project title, user-facing text (window titles, About, docs prose) | **Chrono Calc** (two words, each capitalized) |
| Project identifier, repository name, release file names, executable | `chrono-calc` |
| C++ namespace and other places where hyphens are not allowed | `chrono_calc` |
| The entry in the desktop's application menu (Start menu) | **Calc** |
| Release and delivery file names (CPP-DEVELOPMENT-RULES Rule 4) | `chrono-calc_x.y.z.ext` |

## 3. Technology (fixed, see DECISIONS.md)

- **Language:** C++20.
- **Compilers:** GCC 12 or newer and Clang 15 or newer (Debian 12 and RHEL 9 are the oldest targets).
- **Build:** CMake 3.25 or newer.
- **GUI:** Qt 6 **Widgets**, Qt 6.4 or newer (the Qt of Debian 12 and Ubuntu 24.04). No QML, no KDE Frameworks, no GTK, no webview shells. Qt Widgets looks native on KDE and works on GNOME.
- **Engine:** `CalcManager` and `RatPack` from microsoft/calculator, copied into `third_party/calculator/` with their MIT licence and the commit they came from. They are never edited; anything that must differ is done in our own wrapper (ARCHITECTURE.md).
- **Strictness (the hard gate, like a type check):** the compiler runs with `-Wall -Wextra -Wpedantic -Werror` on our own code; `clang-tidy` with the project's `.clang-tidy` reports zero problems on our own code. `third_party/` is exempt from both.
- **Tests:** Qt Test, run through CTest.
- **Packages:** `.deb` and `.rpm` through CPack; a `.desktop` file named Calc and an AppStream metainfo file. Each package is built in a container of the oldest supported system (Debian 12 for the `.deb`, AlmaLinux 9 for the `.rpm`) and tested in a bare newer one (Ubuntu 24.04, Fedora 43) by the private `build-release.sh` (D-009). Only the owner runs it, for milestone versions.
- **License:** AGPL-3.0-or-later (the same as Chrono Mail). Every dependency must be license-compatible (MIT, LGPL Qt: yes).

## 4. Golden rules

### 4.1 Never a wrong result (outranks everything)

A calculator that gives a wrong answer is worthless.

1. All arithmetic goes through the engine. Our code never computes a result itself (no `double` maths for results).
2. The engine in `third_party/` is never changed. An update of the engine is its own task, with the old and new commit named and every engine test run.
3. Every view is tested against known results of the Windows 7 Calculator for the same key presses (precision, rounding, order of operations, error texts).

### 4.2 Never crash on input

1. Pasted text, odd key sequences, huge numbers, division by zero and invalid input give the calculator's own error display, never a crash.
2. Numbers that come from input (paste, the side panels) are checked before use; sizes are capped.
3. Every crash fix comes with a regression test built from the input that caused it.

### 4.3 Everything is a command

Every user-visible action (a key, a menu item, a mode switch, copy, paste, a side-panel calculation) is a command in `core`, with an id, parameters and tests. The window calls commands and shows their results; it contains no logic of its own.

This also lets an AI agent drive and test every feature without a screen.

### 4.4 Layering is enforced

`third_party` ← `core` ← `ui` ← `app` (ARCHITECTURE.md). A layer only includes from layers below it. `core` is plain C++ without Qt; only `ui` and `app` include Qt Widgets. An automated check enforces this and runs with the tests.

### 4.5 Tests are the gate

1. Every change comes with tests.
2. A task is only done when: the tests related to the change pass (never the full suite, the owner's rule), the build has no warnings, clang-tidy reports nothing, the layer check passes.
3. **A test is unambiguous:** its result never depends on what ran before it or on timing; it answers every dialog it causes; it leaves nothing behind; its name is the same in every run; no test takes longer than 20 seconds.

### 4.6 Security and privacy

1. Chrono Calc needs no network. It never sends anything anywhere.
2. Nothing the user types is logged.

## 5. Code conventions

1. Version number and version history per file, exactly as CPP-DEVELOPMENT-RULES.md Rule 4 describes for C++.
2. The project version lives in one place: the `project(... VERSION x.y.z)` line of the top-level `CMakeLists.txt`. When any code file changes, the project version changes too. A change of only documents keeps the version and is pushed with `./gitpush.sh` without a message.
3. Small files with a single clear responsibility. A code file has at most 2,000 lines.
4. Standard library first, Qt second, own code last.
5. Names are descriptive and spelled out in full. Classes `PascalCase`, functions and variables `camelCase`, files `lowercase.cpp`/`.h`.
6. Every colour lives in one file: `src/core/colours.h`. Nothing takes colours from the platform theme.

## 6. Workflow with AI agents

`CPP-DEVELOPMENT-RULES.md` applies in full. How it maps onto agent work in Claude Code:

1. **"Proceed" = plan approval.** The agent researches and proposes a changelist listing every change (Rule 2). Nothing is edited until the owner writes "Proceed". Approval covers exactly that changelist's scope (Rule 1).
2. **Within an approved changelist**, the agent may build, run tests and fix its own failures, as long as the fixes stay inside the approved scope. Anything outside the scope needs a new changelist.
3. **Delivery.** Files are edited in place. Every response that modifies files ends with the Rule 5 summary and push line. The owner pushes with `./gitpush.sh`.

## 7. Before you finish a task

1. The task passes the gate of 4.5.2.
2. `ROADMAP.md`: update the task's status.
3. `STATUS.md`: rewrite it (current state, next task, open questions).
4. `DEVLOG.md`: append a short entry (what landed, numbers, what is still open).
5. `DECISIONS.md`: add an entry for every design decision made during the task.

Sessions can end abruptly (context limits, crashes). The documents plus a working tree are how the next session picks up. Keep them current after every task, not at the end of a session.
