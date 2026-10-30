# ARCHITECTURE.md

How Chrono Calc is built. The design is in place before the code; parts marked *(planned)* do not exist yet.

## 1. Folders

| Folder | What is in it |
|---|---|
| `third_party/calculator/` | `CalcManager` and `RatPack` from microsoft/calculator, unchanged, with `LICENSE` and `SOURCE.md` (the commit and date they came from) *(planned, task 0.2)* |
| `src/core/` | Plain C++ without Qt: the wrapper around the engine, the command registry, the side-panel calculations, the colours *(planned)* |
| `src/ui/` | Qt Widgets: the main window, the button grids of every view, the side panels, the dialogs *(planned)* |
| `src/app/` | `main.cpp`: starts Qt, builds the window, nothing else *(planned)* |
| `tests/` | Qt Test cases, one file per part, plus the layer check *(planned)* |
| `packaging/` | The `.desktop` file (Calc), the AppStream metainfo, the icon, CPack settings *(planned)* |
| `scratchpad/` | Working scripts of the sessions; git-ignored |

## 2. Layers

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
3. The layer check (a test) reads every `#include` of `src/` and fails on any include that goes up or sideways.

## 3. The engine and its wrapper

1. The engine is `CalculatorManager` (from `CalcManager`) with the arbitrary-precision number library `RatPack`. It is driven by commands (`Command::Command7`, `Command::CommandADD` and so on) and reports back through an interface (`ICalcDisplay`) with the display text, the expression and the history.
2. `core` holds our wrapper: one class implements `ICalcDisplay`, keeps what the engine reports, and offers it to the UI as plain values. No Qt type crosses into `core`.
3. Anything the engine needs from Windows (if any) is supplied by a small shim in `core`, never by editing `third_party/`. Task 0.2 finds out what is needed.
4. The side panels of Windows 7 that the Windows 10 engine does not have (Worksheets, parts of Date calculation) are our own code in `core`, with the formulas written down in DECISIONS.md and tested against results of the Windows 7 Calculator.

## 4. Commands

Every user action is a command in `core` (AGENTS.md 4.3): an id (for example `standard.digit.7`, `view.scientific`, `edit.copy`), its parameters, and a test. The UI only maps keys, clicks and menu items to command ids, and shows the result.

## 5. Settings

What the Windows 7 Calculator remembers (the last view, Digit grouping, the side panel shown) is stored with `QSettings` in `~/.config/chrono-calc/` *(planned)*.
