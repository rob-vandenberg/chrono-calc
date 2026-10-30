# STATUS.md

**Version:** 0.2.7
**Date:** 2026-10-30

## Where we are

M0 is done (0.0.1 to 0.0.4). M1 (the Standard view) is done (0.1.5, 0.1.6). **M2 (the Scientific view) is done** (0.2.7). The owner told Claude to continue on its own through each milestone, pushing after each reviewed step (OR-5), and to move to a new chat when the token estimate passes 500,000 (with Remote Control on and the chat directly in the Chrono Calc group).

- Engine: `third_party/calculator/` (Microsoft's, commit `79e1ac5`, unchanged, D-014), library `chrono_calc_engine`; its 71 tests ported in `tests/engine/`.
- Core: `core/calculator` drives two `CCalcEngine`s, Standard and Scientific (D-017, D-020): Windows 7's one memory, percent (also right after =, D-019), expression line, Inv, angle units, F-E, brackets, regional separators, no digit grouping. `core/commands` (registry), `core/enginetexts`, `core/keys` (characters, keys, Ctrl letters), `core/layout` (both views' measures, buttons, faces), `core/menus` (View, Edit, Help), `core/colours.h` (every colour). `chrono-calc-cli` runs anything without a screen (D-016).
- Window (D-018, D-020): menus with every Windows 7 item (not-built items greyed; Standard, Scientific and Basic built), both views with their size and the view remembered, the display (expression line, M with a hover text, the number smaller when long), the buttons in our own drawing with Windows 7's faces, every Windows 7 key, fixed size, position remembered in `~/.config/chrono-calc/chrono-calc.conf`. Fusion with only our colours, so the desktop theme changes nothing.
- Windows 7 reference: `tests/reference/windows7-standard.md` (36 results) and `windows7-scientific.md` (45 results, layout, keys, look). All pass (`tests/test_reference_standard.cpp`, `tests/test_reference_scientific.cpp`).
- Build and tests: 14 tests; the related ones pass with GCC 15, Clang 21 and on Debian 12 (Qt 6.4).
- Windows 7 reference computer: the session "Chrono Calc remote control" on the owner's Windows computer, reached directly (Remote Control is on in this chat).

## Next task

M3, the Programmer view (ROADMAP.md 3.1), when the owner says so (versions 0.3.z, D-013). The owner may first test 0.2.7.

## Open questions

None. Our own choices, not measured in Windows 7 (D-005): the colours of a button under the mouse and held down, the menu highlight, the greyed label and the angle circle edge. Not tested in Windows 7: whether it remembers the view after a restart (we do, as its settings do).
