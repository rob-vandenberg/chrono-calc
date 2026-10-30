# STATUS.md

**Version:** 0.1.5
**Date:** 2026-10-30

## Where we are

M0 is done (0.0.1 to 0.0.4). M1 (the Standard view, versions 0.1.z) is under way: its core part landed in 0.1.5. The owner told Claude to continue on its own through the end of M1, pushing after each reviewed step (OR-5), and to move to a new chat when the token estimate passes 500,000.

- Engine: `third_party/calculator/` (Microsoft's, commit `79e1ac5`, unchanged, D-014), library `chrono_calc_engine`; its 71 tests ported in `tests/engine/`.
- Core: `core/calculator` drives `CCalcEngine` directly (D-017): Windows 7's one memory, Windows 7 percent, the Windows 7 expression line, the regional separators (`Separators`), and a settle step for the engine's uninitialised decimal separator. `core/commands` (registry), `core/enginetexts` (texts; `sqrt` and `reciproc` as in Windows 7), `core/keys` (typed characters). `chrono-calc-cli` runs anything without a screen (D-016).
- Windows 7 reference data (layout, menus, 20 results): `tests/reference/windows7-standard.md`. All 20 results pass in `tests/test_reference_standard.cpp`.
- Program window: still the empty window of task 0.1.
- Build and tests: 12 test programs pass with GCC 15, Clang 21 and on Debian 12 (Qt 6.4).
- Windows 7 reference: the owner's Windows computer runs the Windows 7 Calculator; a Claude session there with Remote Control on ("Chrono Calc remote control") reads and drives it on request.

## Next task

The window of M1, tasks 1.1 to 1.4, then the rest of 1.5 (ROADMAP.md). What is decided and ready for it:

1. Layout: exactly the measures in `tests/reference/windows7-standard.md` part A (client 264 x 331; display frame 13,13,238,63 with the expression line, the memory mark M and the main number; buttons 43 x 33 at columns 13, 62, 111, 160, 208 and rows 82, 122, 162, 203, 242, 282; equals 43 x 74; zero 92 x 33). Button labels as Windows 7: MC MR MS M+ M- / backspace CE C +/- sqrt / 7 8 9 / % / 4 5 6 * 1/x / 1 2 3 - = / 0 , +; the decimal button shows the regional separator.
2. Look (D-005): our own drawing, very close to Windows 7; every colour in `src/core/colours.h` only (AGENTS.md 5.6).
3. Menus as part B of the reference, all items present; what is not built yet is greyed, never hidden (OR-15 of Chrono Mail; grey out). View > Standard and Basic carry their dot.
4. Keys: digits, `. ,` decimal, `+ - * / =` and Enter, Escape = C, Delete = CE, Backspace, F9 = +/-, `@` = sqrt, `%`, `r` = 1/x, Ctrl+L MC, Ctrl+R MR, Ctrl+M MS, Ctrl+P M+, Ctrl+Q M- (to confirm against Windows 7 in 1.5). Typed characters go through `core/keys`.
5. The window builds `Calculator` with the separators of `QLocale::system()`; it is a fixed size and remembers its position (`QSettings`, `~/.config/chrono-calc/`).
6. Still to check against Windows 7 in 1.5 (through the Windows computer's session): the keyboard keys of point 4; MR with empty memory; whether M shows after storing 0; `2*3=5%` (here 0.05, the engine's rule after =).

## Open questions

None.
