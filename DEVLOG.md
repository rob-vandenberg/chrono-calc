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

## 2026-10-30: 0.0.2, task 0.1 Build skeleton

- CMake (C++20, Qt 6.4+ Widgets and Test), presets `gcc` and `clang` with Ninja, warnings as errors and clang-tidy on our own targets (D-012), tests with a 20-second limit and no screen.
- `src/core/version` (the project version), `src/ui/mainwindow` (an empty window titled Chrono Calc), `src/app/main.cpp`.
- Tests: `test_version`, `test_mainwindow`, `test_layers` (D-011). The layer check was proven on planted faults: a Qt include, a relative `../ui/` include, a lowercase `qstring.h`, a `.hpp` file, and a `../app/` include in ui; a legitimate relative include passes.
- clang-tidy's include check skips Qt's own headers; moc misread a regex literal, so the layer check reads include lines by hand.
- A fresh review found the layer-check gaps above (fixed) and that the release containers have no clang-tidy: `build-release.sh` 0.0.2 builds with it off.
- Built and tested in a Debian 12 container: CMake 3.25.1, GCC 12.2, Qt 6.4.2, no warnings, 3 of 3 tests pass. On this computer: GCC 15.2 and Clang 21.1, 3 of 3 on both.

## 2026-10-30: 0.0.3, task 0.2 Engine import

- Copied 53 files of microsoft/calculator `src/CalcManager` (commit `79e1ac5`) unchanged into `third_party/calculator/`, with `LICENSE` and `SOURCE.md`; left out `pch.h`, `UnitConverter`, `CalculatorVector.h` and the Windows build files (D-014).
- A trial compile showed only two needs: `pch.h`'s Windows headers (replaced by our `shim/pch.h`) and `<cmath>` for `Ratpack/support.cpp` (given by a compiler option). The engine builds with GCC 15, Clang 21 and on Debian 12 (GCC 12).
- Ported Microsoft's 71 engine tests to Qt Test (RationalTest 9, CalcInputTest 37, CalcEngineTests 4, CalculatorManagerTest 21) with the en-US engine texts (108); all pass with GCC, Clang and on Debian 12 (Qt 6.4). Two helpers ported them; a fresh reviewer compared every check and expected value with Microsoft's originals and found them faithful.
- Qt 6.4 has no `QTest::setThrowOnFail`: removed; a failed check in a helper still fails the test. clang-tidy's complexity check now ignores macros (every QCOMPARE hides a branch) instead of NOLINT blocks.
- The layer check also covers `third_party` (no Qt, core, ui or app), proven on two planted faults.
- Open for 0.3: the engine's `GetString(int)` can store a key to a temporary for an unknown text number (Clang warning); our wrapper must supply every text number.

## 2026-10-30: 0.0.4, tasks 0.3 Wrapper and commands, 0.4 Command-line driver

- `core/calculator` wraps the engine (Standard mode), `core/commands` is the command registry, `core/enginetexts` gives a text for every text number the engine can look up (closes the dangling-key path of `GetString(int)`; 108 en-US texts plus 41 of ours), `core/keys` maps typed characters (D-015).
- One memory as in Windows 7: MS replaces it. `chrono-calc-cli` runs key presses and commands without a screen (D-016).
- Windows 7 reference data collected through the Windows computer's Claude session: layout, menus, 20 results (`tests/reference/windows7-standard.md`). 19 of 20 numbers match; differences in percent after multiply/divide, in the expression line and in the decimal separator are for task 1.5.
- A fresh review found no bug and four weak tests (memory replace, CLI exit codes, CLI error text, the start value); all fixed: `memoryPlaces()`, `tests/cli_check.cmake`, the display starts empty until the engine reports.
- 11 tests pass with GCC 15, Clang 21 and on Debian 12.

## 2026-10-30: 0.1.5, M1 core (task 1.5, first part)

- Compared our engine with the 20 Windows 7 reference results: 19 numbers matched; percent after * and / differed (Windows 10's rule), and the expression line differed in form.
- The wrapper now drives `CCalcEngine` directly: the engine's one memory register is Windows 7's memory. Windows 7 percent with the engine's own formula and `Rational`; the Windows 7 expression line; regional separators (D-017).
- Found a fault in Microsoft's engine through the reference test: `m_decimalSeparator` has no start value, so a second engine with a comma separator could read "0,1" as an overflow. Settled in the wrapper after construction, engine unchanged.
- `tests/test_reference_standard.cpp`: all 20 Windows 7 results pass, with a comma separator as on the Windows computer. 12 test programs pass with GCC.

## 2026-10-30: 0.1.6, M1 window and the Windows 7 checks (tasks 1.1 to 1.5)

- The Standard view's window: the menus of Windows 7 with every item (not-built ones greyed, their shortcut shown but not taken), the display, 28 buttons in our own drawing at Windows 7's measures, every key, fixed size, the position remembered (D-018). Layout, menus, keys and colours live in `core`; the window only maps keys, clicks and menu items to commands.
- Checked on the Windows 7 Calculator through the first chat's session (Remote Control is off in this chat): the keys, the memory at 0, percent after = and choosing Standard/Basic again (reference lines 21 to 36), plus colours, fonts and access keys (part G).
- Found and fixed through that check: Alt+1 (View > Standard) cleared the calculator, Windows 7 keeps it; a percent right after = took y/100, Windows 7 takes y * result / 100 (2*3=5% is 0,3) (D-019).
- Four review rounds. Fixed from them: a saved position touching a screen by one pixel was used; a key's flash could lose a mouse click (the flash is now drawing only); the palette started from the desktop theme's; the tooltips' palette; tests sent keys past the real focus path; the gradient stops; fonts recomputed on every paint and unbounded for bitmap fonts; tests for the new percent path.
- Tests: 13 programs; the related 8 (test_calculator, test_reference_standard 39 rows, test_standardview, test_mainwindow 19, test_layers, 3 CLI checks) pass with GCC 15, Clang 21 and on Debian 12 (Qt 6.4). New tests were shown to fail without their fix.

## 2026-10-30: 0.2.7, M2 the Scientific view (task 2.1)

- Measured on the Windows 7 Calculator through the Windows computer's session (now reached directly, Remote Control on): the layout (508 x 331, 52 buttons, the angle units, the bracket counter), Inv labels, keys, view switching, 45 results, the button faces and label fonts (`tests/reference/windows7-scientific.md`).
- Two engines, as Windows 10: Standard and Scientific (precedence, 32 digits); the one memory moves with a view change. Inv, angle units, F-E, brackets; the expression line from the engine with Windows 7's names (sind, powe, powten, `*`, `/`) (D-020).
- Found and fixed through Windows 7: numbers were grouped (1,234) in both views; Windows 7 starts without grouping. The Standard buttons now have Windows 7's faces too (function and memory darker, digits light) and its label sizes.
- Review: three faults, one cause: during an error the engine takes only C and CE, while the wrapper still changed its angle, Inv and F-E switches (a wrong unit after a view change or a key during an error). Fixed at the cause, with a regression test shown to fail before.
- A second review found the same cause in the expression line: a key ignored during an error still changed it. Now one rule in both views: while an error shows, only C and CE reach the engine; Windows 7 confirmed it does exactly that (part F of the reference). Its fault of keeping F-E on inside after C during an error is not copied.
- Tests: all 45 Windows 7 Scientific lines plus switching, error and F-E checks pass (`test_reference_scientific`), the 36 Standard lines still pass; the related 9 tests pass with GCC 15, Clang 21 and on Debian 12 (Qt 6.4).

## 2026-10-30: 0.3.8, M3 the Programmer view (task 3.1)

- Measured on the Windows 7 Calculator: the layout (508 x 413, 45 buttons, the bit display, bases and word sizes), greyed buttons per base, keys, view switching, two's complement, wrapping, refused digits, 30 results (`tests/reference/windows7-programmer.md`).
- A third engine in integer mode (64 bits, precedence); base and word size kept over a view change; what each base allows; the bits from the engine's own binary text; a click on a bit turns it over (D-021).
- Found through Windows 7: the engine always groups Hex, Oct and Bin with spaces (Windows 10); Windows 7 shows them whole: the wrapper shows them whole.
- Review: the number library keeps its precision in constants shared by all engines; the wrapper set them only at start, so after a visit to Programmer a Standard result could be -3e-47 instead of 0, and a 64-bit subtraction 0 instead of 1. Each view's engine now sets its own on every view change, as Windows 10 does; regression test shown to fail before.
- Checked with Windows 7: memory across views (two's complement from Programmer, fractions kept), A-F with Shift and Caps Lock, Scientific letters with Caps Lock. Windows 7's fault with -1,5 recalled in Programmer is not copied.
- The round choices of every view are one kind of control now (`ui/viewparts`, was `scientificparts`).
- Tests: all 30 Windows 7 Programmer lines plus part B pass (`test_reference_programmer`, 53 rows); the related 10 tests pass with GCC 15, Clang 21 and on Debian 12 (Qt 6.4).

## 2026-10-30: 0.3.9, letter keys on keyboards without Latin letters

- Measured on the Windows 7 Calculator with a Russian layout (only for the calculator, restored afterwards): every letter shortcut keeps working by key position (reference part F line 5).
- Chrono Calc read the typed letter, so on Russian, Greek or Arabic its letter keys and Ctrl letters did nothing. Now a letter of another script is read as the US letter at its key's place (D-022); Latin layouts unchanged.
- Two fresh reviews: the position is only read where its code is known (Linux XKB: scan code plus 8; Windows: the scan code); Thai keys that type a mark are covered; with Ctrl and two layouts (German and Russian) Qt gives the other layout's Latin letter, so the layout's own key symbol tells the script. Not measured: keys of another script that type no letter (Greek Q types ;).
- Tests: `aKeyboardWithoutLatinLettersKeepsTheLetterKeys` (shown to fail on 0.3.8) and `eachLetterKeyGivesTheLatinLetterAtItsPlace`; the related tests pass with GCC 15, Clang 21 and on Debian 12 (Qt 6.4).

## 2026-10-30: 0.3.10, signs on letter places of Greek and Hebrew

- Measured on the Windows 7 Calculator with Greek and Hebrew (only for the calculator, restored afterwards): the letter of a key's place wins where the view has a command for it, otherwise the typed sign counts (reference part F line 6).
- Chrono Calc read the Greek Q key's ; as Int and the Hebrew Q key's / as divide in Scientific. Qt does not say which layout typed a sign, so `ui/keyboardlayouts` reads the layouts in use (X11 `_XKB_RULES_NAMES`, Wayland's keyboard description, XKB's defaults in the tests) and tells whether the sign comes from a layout that is not Latin (D-022). Checked on this desktop: the X11 reading finds the us layout.
- Builds need xkbcommon, xcb and wayland-client; `build-release.sh` installs their build packages in Debian 12 and AlmaLinux 9.
- Tests: `test_keyboardlayouts` (Greek, Hebrew, AZERTY, Dvorak, US) and `aSignOnALetterPlaceCountsOnlyWhereTheLetterHasNoCommand` (shown to fail on 0.3.9); the related tests pass with GCC 15, Clang 21 and on Debian 12 (Qt 6.4).

## 2026-10-30: 0.4.11, M4 the Statistics view (task 4.1)

- Measured on the Windows 7 Calculator through the Windows computer's session: the layout (264 x 456, 29 buttons, the list of five rows inside a 188 px display, the count, the ▲ ▼ buttons), keys, the list's selection, focus and Delete, C and CAD, the error, view changes, F-E, 16-digit results of six lists (`tests/reference/windows7-statistics.md`).
- A fourth engine (16 digits, no operators); the list of exact values, Add, CAD, x̄ x̄² Σx Σx² σn σn-1 on the engine's fractions and its own square root (`core/statistics`), "Enter data to calculate"; the list widget and the selection buttons (`ui/statisticsparts`) (D-023).
- Found through Windows 7: Σx of an empty list is "Enter data to calculate", not 0. Windows 7's fault of never deleting the last value is not copied.
- Review: selecting an item was not a command (only the window could do it); now `statistics.select.N`, and the CLI prints the list. A statistic the number library cannot compute shows the engine's overflow.
- Tests: `test_reference_statistics` (53 rows and behaviours), the Statistics parts of `test_mainwindow` and `test_standardview`, `cli_shows_the_statistics_list`; the related 13 tests pass with GCC 15, Clang 21 and on Debian 12 (Qt 6.4). The new tests cannot build on 0.3.10 (no Statistics commands there).

## 2026-10-30: 0.5.12, M5 History, Digit grouping and Edit (task 5.1)

- Measured on the Windows 7 Calculator through the Windows computer's session, in four rounds: History's layout, what it records, the keys, clicks, Edit (F2) with 26 typed texts, Digit grouping in every view, Copy, 39 pasted texts, the Statistics view's Edit menu, the registry values (`tests/reference/windows7-history.md`).
- History: the wrapper's own list per view from what the engine records; a Standard chain is one entry; choosing an entry shows its result as written; Edit reads the text (`core/historyedit`) and calculates it in an evaluation engine; the window grows 125 px with the list (D-024). Digit grouping through the engine with the locale's pattern; Copy, Paste (the text typed as keys), Copy history, Copy dataset; the Statistics Edit; History and Digit grouping remembered.
- Found while building: the engines share one "last shown" cache, so a value from an evaluation engine kept the display from changing; the evaluation engine now ends with a clear.
- Not copied from Windows 7 (D-024): Escape adding a pending function to the history, a Statistics edit that is no number deleting the value, hidden menu items (greyed instead), the 150 ms slide.
- Eight review rounds until one found nothing: keys in the edit box, the wheel under it, focus after Cancel, F2 with History hidden; = hanging when the engine's precedence stack of 25 is full (also typed by hand, before M5); edits deeper than the engine takes or with exponents over four digits; Hex E and -1E in Paste; double grouping in Hex; where a pasted minus is the sign (Hex letters, skipped characters, a capital E, a colon before a line end). All fixed with regression tests.
- Tests: `test_reference_history` (181 rows and behaviours), new parts of `test_mainwindow`, `test_standardview`, `test_calculator`, `cli_pastes_and_keeps_the_history`; the related 15 tests pass with GCC 15, Clang 21 and on Debian 12 (Qt 6.4). The new tests cannot build on 0.4.11 (no History commands there).

## 2026-10-30: M6 Unit conversion (task 6.1), built in chrono-calc (5); pushed with M7 and M8 as 0.8.13

- Measured on the Windows 7 Calculator through the Windows computer's session, in one request: the panel's layout in every view, its colours, the 11 types and 105 units in order, 1 of each type's first unit in every unit, 43 other results, what the From box takes, unit and type changes, keys, Tab order, focus, Copy, the registry (`tests/reference/windows7-unitconversion.md`).
- The side panels as one family (D-025): `SidePanel` in the calculator, `view.basic`/`view.unit_conversion` with one dot, the panel beside every view (`core/layout` `sidePanelLayoutOf`), its frame, lists and boxes in `ui/sidepanels`, the panel shown remembered (`window/sidePanel`).
- Unit conversion: `core/unitconversion` (each unit's exact size in the type's base unit, converted with the engine's `Rational`), `core/unitconverter` (type, units, From text, result), the commands `unit.type.N`, `unit.from.N`, `unit.to.N`, `unit.input`; the To box is the engine's own 16-digit writing. A probe with the engine matched every Windows 7 cell of part C1 at once, except five where Windows 7's rounded factors are wrong (mmHg 133.3 Pa, psi 6894.757 Pa, the 2002 electronvolt, a cut foot-pound): not copied (D-025).
- Tests written, not run (the owner's order: the tests of M6, M7 and M8 run together at the end, in chrono-calc (7)): `test_reference_unitconversion` (the lists, part C1 and C2, the From box, unit and type changes, view changes, the Programmer base), four new parts of `test_mainwindow` (sizes in every view, converting, keys and Tab order, remembered), `cli_shows_the_unit_conversion`. Built warning-free with GCC and Clang, clang-tidy clean.

## 2026-10-30: M7 Date calculation (task 7.1), built in chrono-calc (6); pushed with M6 and M8 as 0.8.13

- Measured on the Windows 7 Calculator through the Windows computer's session, in one request and one follow-up: the panel's layout in both modes and every view, its colours, the two modes, the date boxes (format, range 1601 to 9999, keys), 24 differences, 30 Add or subtract results, the order of years, months and days, the number boxes, keys, Tab order, focus, what is kept, the registry (`tests/reference/windows7-datecalculation.md`).
- `core/calendar` (Gregorian day numbers), `core/datecalculation` (difference, add and subtract, the texts), `core/datecalculator` (the panel's state and commands), `DateCalculationPanel` in `ui/sidepanels` with its date boxes, number boxes, arrows and Calculate button, `view.date_calculation` (Ctrl+E), the panel name kept in `window/sidePanel` generally ("view." + name). A scratch check of the calendar code matched all 54 Windows 7 results before the tests were written.
- Not copied from Windows 7 (D-026): the calendar opened and closed without a change emptying the results.
- Tests written, not run (the owner's order): `test_reference_datecalculation` (part D and E, start at today, emptying, the number boxes, the range, what is kept, the ids), six new parts of `test_mainwindow` (beside every view and the focus, both modes, the number boxes' keys, the arrows, Tab order, remembered), `cli_shows_the_date_difference`, `cli_shows_the_shifted_date`. Built warning-free with GCC and Clang, clang-tidy clean.

## 2026-10-30: 0.8.13, M8 Worksheets (task 8.1), built in chrono-calc (7); the tests of M6, M7 and M8 run together

- Measured on the Windows 7 Calculator through the Windows computer's session, in one request and one follow-up: the panel's layout (the same boxes, relabelled per value), colours, the four lists, 115 results (Mortgage 40, Vehicle lease 37, Fuel economy 38), how numbers are read and written (16 digits, exponents), the balloons and their order, keys, Tab order, focus, what is kept, the registry (`tests/reference/windows7-worksheets.md`).
- `core/worksheets` (the four worksheets and their formulas on the engine's `Rational`, `Pow` and `Log`), `core/worksheetcalculator` (each worksheet's chosen value, texts and message; Windows 7's number reading; a result taken when two precisions agree), `WorksheetPanel` and `PanelBalloon` in `ui/sidepanels`, `view.mortgage`, `view.vehicle_lease`, `view.fuel_economy_mpg`, `view.fuel_economy_litres`, `worksheet.choose.N`, `worksheet.value.N`, `worksheet.calculate`; the CLI prints a worksheet. The number boxes' typed-text filter is shared with Unit conversion (`acceptedNumberText`, `typedNumber` in `core/unitconversion`). A scratch check matched all 57 money results of the first answer to the exact formulas before any code, and all 130 Windows 7 cells through the CLI on the first build.
- Not copied from Windows 7 (D-027): "Unable to calculate" for an interest rate of 0 (the payment is the loan divided by the number of payments).
- The tests of M6, M7 and M8 ran for the first time: four faults found and fixed. 1 radian in degrees is 57,29577951308232 (Windows 7's last digit 3 is wrong; D-025, the reference and its test say so). The window tests found replaced panels that were still waiting for deletion (`partOf` lets them go first), the M6 Tab test read Qt's focus chain instead of pressing Tab, and two menu tests still expected the side panels greyed.
- Review, round 1 (four reviewers, one per milestone and M8's window): M8 results below 1 with an exponent lost digits (the engine's exponent form counts the zeros after the decimal symbol; `writtenNumber` writes the mantissa itself), terms beyond 10 million years gave no answer, Escape on the list kept the focus, the balloon outlived a value change and a window move; M7 Enter with a waiting digit gave empty results, a window test could fail at midnight; M6 the BTU was Windows 7's rounded 1055.056 J (exact: 1055.05585262), and From is read like the Worksheets' boxes (measured: part G of the M6 reference), with a To in exponent form when From has one, and a 26th character refused. Round 2: (1+i)^-n with more than 2^31 periods at a tiny rate was taken as negligible (a wrong payment), a result beyond 10^43429 ended the program, a pasted text was cut before it was filtered, tests missing or not guarding their fix under every locale. All fixed, each with a test.
- The related 13 tests pass with GCC 15, Clang 21 and on Debian 12 (Qt 6.4): `test_reference_unitconversion`, `test_reference_datecalculation`, `test_reference_worksheets`, `test_mainwindow`, `test_calculator`, `test_standardview`, `test_version`, `test_layers`, the five panel CLI tests.
