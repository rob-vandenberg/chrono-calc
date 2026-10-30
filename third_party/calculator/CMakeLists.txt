# --- Version ------------------------------------------------------------
# third_party/calculator/CMakeLists.txt 0.0.3

# --- Version history ----------------------------------------------------
# v0.0.3: Initial: Microsoft's engine core as the library chrono_calc_engine (D-014)

# Microsoft's code, unchanged (SOURCE.md): no warnings-as-errors and no clang-tidy here (AGENTS.md 3)
add_library(chrono_calc_engine STATIC
    CalcManager/CalculatorHistory.cpp
    CalcManager/CalculatorManager.cpp
    CalcManager/ExpressionCommand.cpp
    CalcManager/NumberFormattingUtils.cpp
    CalcManager/CEngine/CalcInput.cpp
    CalcManager/CEngine/CalcUtils.cpp
    CalcManager/CEngine/History.cpp
    CalcManager/CEngine/Number.cpp
    CalcManager/CEngine/Rational.cpp
    CalcManager/CEngine/RationalMath.cpp
    CalcManager/CEngine/calc.cpp
    CalcManager/CEngine/scicomm.cpp
    CalcManager/CEngine/scidisp.cpp
    CalcManager/CEngine/scifunc.cpp
    CalcManager/CEngine/scioper.cpp
    CalcManager/CEngine/sciset.cpp
    CalcManager/Ratpack/basex.cpp
    CalcManager/Ratpack/conv.cpp
    CalcManager/Ratpack/exp.cpp
    CalcManager/Ratpack/fact.cpp
    CalcManager/Ratpack/itrans.cpp
    CalcManager/Ratpack/itransh.cpp
    CalcManager/Ratpack/logic.cpp
    CalcManager/Ratpack/num.cpp
    CalcManager/Ratpack/rat.cpp
    CalcManager/Ratpack/support.cpp
    CalcManager/Ratpack/trans.cpp
    CalcManager/Ratpack/transh.cpp
)

# Users include the engine as "CalcManager/CalculatorManager.h"; the engine's own files include each other from CalcManager/
target_include_directories(chrono_calc_engine
    SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/CalcManager
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/shim
)

# support.cpp uses log2 and ceil without including <cmath>; the header is given here so the file stays unchanged
set_source_files_properties(CalcManager/Ratpack/support.cpp PROPERTIES COMPILE_OPTIONS "-include;cmath")
