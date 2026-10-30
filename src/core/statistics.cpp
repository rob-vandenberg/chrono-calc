#include "core/statistics.h"

#include "Header Files/Rational.h"
#include "Header Files/RationalMath.h"

#include <cstdint>
#include <optional>
#include <span>

// --- Version ------------------------------------------------------------
// src/core/statistics.cpp 0.4.11

// --- Version history ----------------------------------------------------
// v0.4.11: Initial: the statistic functions of the Windows 7 Statistics view, on the engine's exact fractions (D-010)

namespace chrono_calc {

using CalcEngine::Rational;

namespace {

Rational sumOf(std::span<const Rational> values)
{
    Rational sum(0);
    for (const Rational& value : values) {
        sum += value;
    }
    return sum;
}

Rational sumOfSquaresOf(std::span<const Rational> values)
{
    Rational sum(0);
    for (const Rational& value : values) {
        sum += value * value;
    }
    return sum;
}

// The sum of the squared distances from the mean, exact: Σ(x - x̄)²
Rational squaredDeviations(std::span<const Rational> values)
{
    const Rational mean = sumOf(values) / Rational(static_cast<uint64_t>(values.size()));
    Rational sum(0);
    for (const Rational& value : values) {
        const Rational distance = value - mean;
        sum += distance * distance;
    }
    return sum;
}

// The engine's own square root (the √ button: RationalMath::Root(x, 2))
Rational squareRoot(const Rational& value)
{
    return CalcEngine::RationalMath::Root(value, Rational(2));
}

}  // namespace

std::optional<Rational> statisticOf(Statistic statistic, std::span<const Rational> values)
{
    const auto count = Rational(static_cast<uint64_t>(values.size()));
    // The number library reports what it cannot compute (an overflow) by throwing its error code
    try {
        switch (statistic) {
        case Statistic::Sum:
            return sumOf(values);
        case Statistic::SumOfSquares:
            return sumOfSquaresOf(values);
        case Statistic::Mean:
            return values.empty() ? std::nullopt : std::optional(sumOf(values) / count);
        case Statistic::MeanOfSquares:
            return values.empty() ? std::nullopt : std::optional(sumOfSquaresOf(values) / count);
        case Statistic::PopulationDeviation:
            return values.empty() ? std::nullopt : std::optional(squareRoot(squaredDeviations(values) / count));
        case Statistic::SampleDeviation:
            return values.size() < 2 ? std::nullopt
                                     : std::optional(squareRoot(squaredDeviations(values) / (count - Rational(1))));
        }
    } catch (const uint32_t /*error*/) {
        return std::nullopt;
    }
    return std::nullopt;
}

}  // namespace chrono_calc
