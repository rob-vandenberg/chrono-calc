# STATUS.md

**Version:** 0.4.11
**Date:** 2026-10-30

## Where we are

M0 is done (0.0.1 to 0.0.4). M1 (the Standard view) is done (0.1.5, 0.1.6). M2 (the Scientific view) is done (0.2.7). M3 (the Programmer view) is done (0.3.8); 0.3.9 and 0.3.10 keep the letter keys working on keyboards without Latin letters as Windows 7 does (D-022). **M4 (the Statistics view) is done** (0.4.11, D-023). The owner told Claude to continue on its own through each milestone, pushing after each reviewed step (OR-5), and to move to a new chat when the token estimate passes 500,000 (with Remote Control on and the chat directly in the Chrono Calc group).

- Engine: `third_party/calculator/` (Microsoft's, commit `79e1ac5`, unchanged, D-014), library `chrono_calc_engine`; its 71 tests ported in `tests/engine/`.
- Core: `core/calculator` drives four `CCalcEngine`s, Standard, Scientific, Programmer and Statistics (D-017, D-020, D-021, D-023): Windows 7's one memory, percent (also right after =, D-019), expression line, Inv, angle units, F-E, brackets, bases, word sizes, the bits, the Statistics list and its statistics (`core/statistics`, exact fractions), regional separators, no digit grouping. `core/commands` (registry), `core/enginetexts`, `core/keys` (characters, keys, Ctrl letters), `core/layout` (both views' measures, buttons, faces), `core/menus` (View, Edit, Help), `core/colours.h` (every colour). `chrono-calc-cli` runs anything without a screen (D-016).
- Window (D-018, D-020): menus with every Windows 7 item (not-built items greyed; the four views and Basic built), the four views with their size and the view remembered, the bit display, the Statistics list with its count and selection buttons (`ui/statisticsparts`), the display (expression line, M with a hover text, the number smaller when long), the buttons in our own drawing with Windows 7's faces, every Windows 7 key, fixed size, position remembered in `~/.config/chrono-calc/chrono-calc.conf`. Fusion with only our colours, so the desktop theme changes nothing.
- Windows 7 reference: `tests/reference/windows7-standard.md` (36 results) `windows7-scientific.md` (45 results), `windows7-programmer.md` (30 results and the behaviours) and `windows7-statistics.md` (the results of six lists and the behaviours). All pass (`tests/test_reference_*.cpp`).
- Keys on Russian, Greek, Hebrew or Arabic keyboards: a key is read by its place first, its sign only where the view has no command for that letter, as Windows 7 does (measured, reference part F lines 5 and 6); the layouts in use are read from X11 or Wayland (`ui/keyboardlayouts`).
- Build and tests: 18 tests (builds need xkbcommon, xcb and wayland-client; `build-release.sh` installs them); the related ones pass with GCC 15, Clang 21 and on Debian 12 (Qt 6.4).
- Windows 7 reference computer: the session "Chrono Calc remote control" on the owner's Windows computer, reached directly (Remote Control is on in this chat).

## Next task

M5, History, Digit grouping and Edit (ROADMAP.md 5.1), when the owner says so; the owner tests 0.4.11 first.

## Open questions

For M9 (ROADMAP 9.2): the Wayland reading of the keyboard layouts (D-022) has not run on a real Wayland desktop; the owner tests it with the `.rpm` on a Fedora computer with Wayland.

None. Our own choices, not measured in Windows 7 (D-005): the colours of a button under the mouse and held down, the menu highlight, the greyed label, the angle circle edge, the glow of the ▲ ▼ buttons and the place of the thin line above the count. Not tested in Windows 7: whether it remembers the view after a restart (we do, as its settings do).
