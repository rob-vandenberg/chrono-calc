#pragma once

#include "core/view.h"

#include "Header Files/Rational.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/worksheets.h 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the four Worksheets of Windows 7 (Mortgage, Vehicle lease, Fuel economy in mpg and in L/100 km),
//          their values and their formulas on the engine's exact fractions (D-010, D-027)

namespace chrono_calc {

// The four Worksheets, in the order of View > Worksheets
enum class Worksheet : std::uint8_t
{
    Mortgage,
    VehicleLease,
    FuelEconomyMpg,
    FuelEconomyLitres,
};

inline constexpr std::size_t worksheetCount = 4;

// One worksheet (tests/reference/windows7-worksheets.md part A line 3, part B line 1): its values in their own order,
// which is the order of the boxes (the value being calculated has no box, the others keep this order); the list of the
// values that can be calculated, in the list's (sorted) order, as indexes into "values"; the value calculated the first
// time
struct WorksheetSpec
{
    std::string_view name;
    std::vector<std::string_view> values;
    std::vector<std::size_t> choices;
    std::size_t firstChoice = 0;
};

const std::array<WorksheetSpec, worksheetCount>& worksheets();
const WorksheetSpec& worksheetSpec(Worksheet worksheet);
// The worksheet a side panel shows; nothing for Basic, Unit conversion and Date calculation
std::optional<Worksheet> worksheetOf(SidePanel panel);

// The value "chosen" of the worksheet from the others ("values" holds one number per value of the worksheet, in its own
// order; the chosen one is not read), on the engine's exact fractions with its own powers and logarithm (D-010).
// Nothing when there is no answer: a division by zero, a payment that never repays the loan (Windows 7: "Unable to
// calculate"). The number library's radix and precision are global: the caller sets them first (decimal), and the
// result is only as exact as that precision; WorksheetCalculator takes a result two precisions agree on (D-027).
std::optional<CalcEngine::Rational> calculateWorksheet(Worksheet worksheet, std::size_t chosen,
                                                       const std::vector<CalcEngine::Rational>& values);

}  // namespace chrono_calc
