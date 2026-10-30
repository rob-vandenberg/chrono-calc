# ARCHITECTURE.md

How Chrono Calc is built. The design is in place before the code; parts marked *(planned)* do not exist yet.

## 1. Folders

| Folder | What is in it |
|---|---|
| `third_party/calculator/` | The engine core from microsoft/calculator commit `79e1ac5`, unchanged (`CalcManager/`), with `LICENSE`, `SOURCE.md` (what was copied, left out and why), our `shim/pch.h` and `CMakeLists.txt` (D-014) |
| `src/core/` | Plain C++ without Qt: `version` (the project version), `calculator` (the engine wrapper, D-015), `commands` (the command registry), `enginetexts` (the engine's texts), `keys` (typed characters and keys), `statistics` (the Statistics view's functions on exact fractions), `historyedit` (reads an edited history entry or value), `unitconversion` (the unit types and units, converted on exact fractions), `unitconverter` (the Unit conversion panel's state), `calendar` (Gregorian day numbers), `datecalculation` (the date difference, add and subtract), `datecalculator` (the Date calculation panel's state), `worksheets` (the four Worksheets and their formulas), `worksheetcalculator` (the Worksheets' state and Windows 7's number reading), `layout` (every view's measures and buttons, their faces and label sizes), `view.h` (the views, angle units, number bases, word sizes), `menus` (View, Edit, Help), `colours` (every colour) |
| `src/ui/` | Qt Widgets: `mainwindow` (the window: menus, keys, the Standard view, the remembered position), `calcbutton` (our button drawing), `displaypanel` (the display), `look` (Fusion with our colours only), `fonts` (fonts sized by their digits), `viewparts` (the round choices, the bracket counter, the bit display), `statisticsparts` (the Statistics list, also the history list, its selection buttons and the edit box over a row), `keyboardlayouts` (which keyboard layout a key comes from: XKB, X11, Wayland; Linux only, a Windows build would leave it out), `sidepanels` (the panel beside the calculator, its lists, boxes, date boxes and button, the Unit conversion and Date calculation panels, the Worksheet panel and its message balloon); later the dialogs |
| `src/app/` | `main.cpp`: starts Qt, sets the look and the settings name, builds the window, nothing else; `cli.cpp`: `chrono-calc-cli`, the calculator without a screen (D-016) |
| `tests/` | Qt Test cases, one program per file, run with CTest (20 seconds at most, no screen): `test_version`, `test_mainwindow` (the window, keys, menus, position), `test_calculator`, `test_standardview` (both views' buttons and menus), `test_reference_standard`, `test_reference_scientific`, `test_reference_programmer`, `test_reference_statistics`, `test_reference_history`, `test_reference_unitconversion`, `test_reference_datecalculation` and `test_reference_worksheets` (Windows 7 results), `test_keyboardlayouts` (Latin or not), `test_layers` (the layer check); `tests/engine/`: Microsoft's 71 engine tests ported to Qt Test |
| `packaging/` | The `.desktop` file (Calc), the AppStream metainfo, the icon, CPack settings *(planned)* |
| `scratchpad/` | Working scripts of the sessions; git-ignored |

## 2. Building

1. `cmake --preset gcc` and `cmake --build --preset gcc` (or `clang`); `ctest --preset gcc` runs the tests. The build lands in `build/gcc` or `build/clang`.
2. Our own targets compile with warnings as errors and run clang-tidy during the build (`chrono_calc_strict()` in the top-level `CMakeLists.txt`, D-012). `third_party/` never gets either.
3. The executable is `chrono-calc`; the libraries are `chrono_calc_engine` (Microsoft's engine, no warnings-as-errors, no clang-tidy), `chrono_calc_core` and `chrono_calc_ui`.

## 3. Layers

```
app          main.cpp
 ↑
ui           Qt Widgets: windows, button grids, side panels
 ↑
core         plain C++: engine wrapper, commands, conversions, dates, worksheets, colours
 ↑
third_party  microsoft/calculator engine (unchanged)
```

1. A layer includes only from the layers below it.
2. `core` and `third_party` never include Qt. Only `ui` and `app` do.
3. The layer check (`tests/test_layers.cpp`, D-011) reads every `#include` of `src/` and `third_party/` and fails on any include that goes up or sideways.

## 4. The engine and its wrapper

1. The engine is `CalculatorManager` (from `CalcManager`) with the arbitrary-precision number library `RatPack`. It is driven by commands (`Command::Command7`, `Command::CommandADD` and so on) and reports back through an interface (`ICalcDisplay`) with the display text, the expression and the history.
2. `core` holds our wrapper, `Calculator` (`core/calculator.h`), with one engine per view (D-020, D-023; the Statistics view's list and statistics are the wrapper's own) and two evaluation engines that calculate an edited history entry without showing it (D-024); the history lists are the wrapper's, filled from what the engines record: its inner class `Display` implements `ICalcDisplay` and keeps what the engine reports; `Calculator` offers it as plain values (`display()`, `expression()`, `isError()`, `hasMemory()`) and runs commands by id (`run()`). No Qt type crosses into `core`.
3. What the engine needs from Windows is supplied outside Microsoft's files, never by editing them: `third_party/calculator/shim/pch.h` and a compiler option for `Ratpack/support.cpp` (D-014).
4. The side panels of Windows 7 that the Windows 10 engine does not have (Worksheets, parts of Date calculation) are our own code in `core`, with the formulas written down in DECISIONS.md and tested against results of the Windows 7 Calculator.

## 5. Commands

Every user action is a command in `core` (AGENTS.md 4.3): an id (for example `standard.digit.7`, `view.scientific`, `edit.copy`), its parameters, and a test. The UI only maps keys, clicks and menu items to command ids, and shows the result.

## 6. Settings

What the Windows 7 Calculator remembers (the last view, Digit grouping, History shown, the side panel shown) is stored with `QSettings` in `~/.config/chrono-calc/chrono-calc.conf`: the window's position (D-018), its view (D-020), History shown and Digit grouping (D-024), the side panel shown (D-025, `window/sidePanel`).
