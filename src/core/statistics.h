#pragma once

#include "Header Files/Rational.h"

#include <cstdint>
#include <optional>
#include <span>

// --- Version ------------------------------------------------------------
// src/core/statistics.h 0.4.11

// --- Version history ----------------------------------------------------
// v0.4.11: Initial: the statistic functions of the Windows 7 Statistics view, on the engine's exact fractions (D-010)

namespace chrono_calc {

// The statistic functions of the Statistics view (Windows 7: x̄, x̄², Σx, Σx², σn, σn-1)
enum class Statistic : std::uint8_t
{
    Mean,
    MeanOfSquares,
    Sum,
    SumOfSquares,
    PopulationDeviation,
    SampleDeviation,
};

// The statistic of the values, computed with the engine's exact fractions (Rational) and its own square root, never
// with floating point (D-010). Nothing where it is not defined: no values for the means and deviations, one value for
// σn-1, or a number the engine cannot hold; the sums of no values are 0.
std::optional<CalcEngine::Rational> statisticOf(Statistic statistic, std::span<const CalcEngine::Rational> values);

}  // namespace chrono_calc
