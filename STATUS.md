# STATUS.md

**Version:** 0.1.6
**Date:** 2026-10-30

## Where we are

M0 is done (0.0.1 to 0.0.4). **M1 (the Standard view) is done** (0.1.5 core, 0.1.6 window and the Windows 7 checks). The owner told Claude to continue on its own through the end of M1, pushing after each reviewed step (OR-5), and to move to a new chat when the token estimate passes 500,000.

- Engine: `third_party/calculator/` (Microsoft's, commit `79e1ac5`, unchanged, D-014), library `chrono_calc_engine`; its 71 tests ported in `tests/engine/`.
- Core: `core/calculator` drives `CCalcEngine` directly (D-017): Windows 7's one memory, percent (also right after =, D-019), expression line, regional separators. `core/commands` (registry), `core/enginetexts`, `core/keys` (characters, keys, Ctrl letters), `core/standardlayout` (measures and buttons), `core/menus` (View, Edit, Help), `core/colours.h` (every colour). `chrono-calc-cli` runs anything without a screen (D-016).
- Window (D-018): menus with every Windows 7 item (not-built items greyed, only Standard and Basic built), the display (expression line, M with a hover text, the number smaller when long), 28 buttons in our own drawing, every Windows 7 key, fixed size, position remembered in `~/.config/chrono-calc/chrono-calc.conf`. Fusion with only our colours, so the desktop theme changes nothing.
- Windows 7 reference: `tests/reference/windows7-standard.md` parts A to G (layout, menus, 36 results, keys, memory at 0, percent after =, look). All 36 results pass (`tests/test_reference_standard.cpp`).
- Build and tests: 13 test programs; the related ones pass with GCC 15, Clang 21 and on Debian 12 (Qt 6.4).
- Windows 7 reference computer: reached through the first chat's session "chrono-calc (0)", which forwards requests to "Chrono Calc remote control" (this chat's Remote Control is off).

## Next task

M2, the Scientific view (ROADMAP.md 2.1), when the owner says so; the owner decides whether the version becomes 0.2.z (D-013). The owner may first test 0.1.6 and decide on a milestone release (`./build-release.sh`, the owner's).

## Open questions

None. Our own choices, not measured in Windows 7 (D-005): the colours of a button under the mouse and held down (Windows 7 fades to a warm tone; only a mid-fade sample was measured), and the menu highlight.
