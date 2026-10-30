# SOURCE.md: where the engine comes from

The calculator engine in `CalcManager/` is Microsoft's, copied **unchanged** (D-002, D-014).

| | |
|---|---|
| Repository | https://github.com/microsoft/calculator |
| Commit | `79e1ac5` (2026-10-20, "Harden snapshot replay and integer powers") |
| Licence | MIT, `LICENSE` in this folder (copied unchanged) |
| Copied on | 2026-10-30, from the reference clone `/home/rob/c++/calculator` |

## What was copied

From `src/CalcManager/`, with the same folder layout:

1. `CEngine/` (12 files), `Ratpack/` (15 files), `Header Files/` (12 files).
2. `CalculatorManager`, `CalculatorHistory`, `ExpressionCommand`, `NumberFormattingUtils` (`.cpp` and `.h`).
3. `CalculatorResource.h`, `Command.h`, `ExpressionCommandInterface.h`, `sal_cross_platform.h`, `winerror_cross_platform.h`.

## What was left out, and why

1. `pch.h`: it includes the Windows-only headers `intsafe.h`, `ppltasks.h` and `winerror.h`. Replaced by our own `shim/pch.h`.
2. `UnitConverter.cpp`, `UnitConverter.h`: they need the Windows-only PPL tasks, and their unit lists are Windows 10's (D-010).
3. `CalculatorVector.h`: no file uses it.
4. `pch.cpp`, `ratpak.natvis`, `CalcManager.vcxproj`, `CalcManager.vcxproj.filters`: Windows build files.

## What is ours (not Microsoft's)

1. `shim/pch.h`: the standard headers of Microsoft's `pch.h`, without the Windows-only ones.
2. `CMakeLists.txt`: builds the library `chrono_calc_engine`. It gives `Ratpack/support.cpp` the standard header `<cmath>` through a compiler option, because that file uses `log2` and `ceil` without including it; the file itself stays unchanged.

## Updating the engine

An update is its own task (AGENTS.md 4.1.2): name the old and the new commit here, copy the same files again, and run every engine test in `tests/engine/`.
