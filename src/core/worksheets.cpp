#include "core/worksheets.h"

#include "core/view.h"

#include "Header Files/Rational.h"
#include "Header Files/RationalMath.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/worksheets.cpp 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the four Worksheets of Windows 7 (Mortgage, Vehicle lease, Fuel economy in mpg and in L/100 km),
//          their values and their formulas on the engine's exact fractions (D-010, D-027)

namespace chrono_calc {

using CalcEngine::Rational;

namespace {

// The values of each worksheet, in the order of its boxes (windows7-worksheets.md part A line 3)
namespace mortgage {
constexpr std::size_t price = 0;
constexpr std::size_t down = 1;
constexpr std::size_t term = 2;
constexpr std::size_t rate = 3;
constexpr std::size_t payment = 4;
}  // namespace mortgage

namespace lease {
constexpr std::size_t value = 0;
constexpr std::size_t period = 1;
constexpr std::size_t perYear = 2;
constexpr std::size_t residual = 3;
constexpr std::size_t rate = 4;
constexpr std::size_t payment = 5;
}  // namespace lease

namespace fuel {
constexpr std::size_t distance = 0;
constexpr std::size_t used = 1;
constexpr std::size_t economy = 2;
}  // namespace fuel

const Rational zero(0);
const Rational one(1);

// A series stops when its next term no longer reaches the last of the 128 digits the number library's own arithmetic
// keeps (RATIONAL_PRECISION)
bool negligible(const Rational& term, const Rational& sum)
{
    static const Rational scale = CalcEngine::RationalMath::Pow(Rational(10), Rational(130));
    return CalcEngine::RationalMath::Abs(term) * scale <= CalcEngine::RationalMath::Abs(sum);
}

// ln(1 + y) for y above -1. Near 0 the series y - y²/2 + y³/3 - ..., because 1 + y would lose y's digits (the engine's
// power and logarithm take a number within their precision of 1 as 1); the engine's logarithm otherwise
Rational logOnePlus(const Rational& y)
{
    if (CalcEngine::RationalMath::Abs(y) * Rational(1000) >= one) {
        return CalcEngine::RationalMath::Log(one + y);
    }
    Rational power = y;
    Rational sum = zero;
    for (int32_t k = 1;; ++k) {
        const Rational term = power / Rational(k);
        sum += (k % 2 == 1) ? term : -term;
        if (negligible(term, sum)) {
            return sum;
        }
        power *= y;
    }
}

// e^x - 1: the series x + x²/2! + x³/3! + ... below 1/2, where e^x - 1 would cancel; the engine's exponential otherwise
Rational expMinusOne(const Rational& x)
{
    if (CalcEngine::RationalMath::Abs(x) * Rational(2) >= one) {
        return CalcEngine::RationalMath::Exp(x) - one;
    }
    Rational term = x;
    Rational sum = x;
    for (int32_t k = 2; !negligible(term, sum); ++k) {
        term = term * x / Rational(k);
        sum += term;
    }
    return sum;
}

// For a rate i and n periods: the share of a loan that n payments repay, 1 - (1 + i)^-n, and what is left of a value
// after n periods, (1 + i)^-n. Worked as e^-(n ln(1 + i)) with the series above, so that a tiny rate keeps its digits and
// any n works (the engine's power refuses a whole n above 2^31). What is left is nothing when it is below e^-100000
// (about 1e-43429): far below what 16 digits of the other terms show.
struct Discounting
{
    Rational repaid;
    std::optional<Rational> left;
};

Discounting discounting(const Rational& rate, const Rational& periods)
{
    static const Rational smallestExponent(-100000);
    const Rational exponent = -periods * logOnePlus(rate);
    if (exponent < smallestExponent) {
        return {.repaid = one, .left = std::nullopt};
    }
    if (CalcEngine::RationalMath::Abs(exponent) * Rational(2) < one) {
        const Rational repaid = -expMinusOne(exponent);
        return {.repaid = repaid, .left = one - repaid};
    }
    const Rational left = CalcEngine::RationalMath::Exp(exponent);
    return {.repaid = one - left, .left = left};
}

// What a payment of 1 at the end of each of n periods is worth now, at the rate i per period: (1 - (1 + i)^-n) / i,
// and n itself at a rate of 0
Rational presentValueOfPayments(const Rational& rate, const Rational& periods)
{
    if (rate == zero) {
        return periods;
    }
    return discounting(rate, periods).repaid / rate;
}

// The number of periods n with (1 + i)^-n = 1 + change: -ln(1 + change) / ln(1 + i). Nothing when 1 + change is 0 or
// less: the payments never repay the loan.
std::optional<Rational> periodsUntil(const Rational& rate, const Rational& change)
{
    if (change <= -one) {
        return std::nullopt;
    }
    return -logOnePlus(change) / logOnePlus(rate);
}

// Mortgage (part C line 1): a monthly rate of rate / 1200, term * 12 monthly payments at the end of each month, a loan of
// the price less the down payment
std::optional<Rational> calculateMortgage(std::size_t chosen, const std::vector<Rational>& values)
{
    const Rational rate = values.at(mortgage::rate) / Rational(1200);
    const Rational periods = values.at(mortgage::term) * Rational(12);
    const Rational& payment = values.at(mortgage::payment);
    if (chosen == mortgage::term) {
        const Rational loan = values.at(mortgage::price) - values.at(mortgage::down);
        if (payment == zero) {
            return std::nullopt;
        }
        if (rate == zero) {
            return loan / payment / Rational(12);
        }
        const std::optional<Rational> months = periodsUntil(rate, -(loan * rate / payment));
        return months ? std::optional<Rational>(*months / Rational(12)) : std::nullopt;
    }
    if (periods == zero) {
        return std::nullopt;
    }
    const Rational factor = presentValueOfPayments(rate, periods);
    switch (chosen) {
    case mortgage::payment:
        return (values.at(mortgage::price) - values.at(mortgage::down)) / factor;
    case mortgage::down:
        return values.at(mortgage::price) - (payment * factor);
    case mortgage::price:
        return values.at(mortgage::down) + (payment * factor);
    default:
        return std::nullopt;
    }
}

// Vehicle lease (part C line 2): a rate of rate / (100 * payments per year) per period, period * payments per year
// payments at the end of each period; the residual value is paid back at the end
std::optional<Rational> calculateLease(std::size_t chosen, const std::vector<Rational>& values)
{
    const Rational& perYear = values.at(lease::perYear);
    if (perYear == zero) {
        return std::nullopt;
    }
    const Rational rate = values.at(lease::rate) / (Rational(100) * perYear);
    const Rational periods = values.at(lease::period) * perYear;
    const Rational& value = values.at(lease::value);
    const Rational& residual = values.at(lease::residual);
    const Rational& payment = values.at(lease::payment);
    if (chosen == lease::period) {
        if (rate == zero) {
            return payment == zero ? std::nullopt : std::optional<Rational>((value - residual) / payment / perYear);
        }
        // payment = (value - residual * (1 + i)^-n) * i / (1 - (1 + i)^-n), solved for (1 + i)^-n
        const Rational divisor = (residual * rate) - payment;
        if (divisor == zero) {
            return std::nullopt;
        }
        // (1 + i)^-n = (value * i - payment) / divisor, which is 1 + i * (value - residual) / divisor
        const std::optional<Rational> count = periodsUntil(rate, rate * (value - residual) / divisor);
        return count ? std::optional<Rational>(*count / perYear) : std::nullopt;
    }
    if (periods == zero) {
        return std::nullopt;
    }
    const Rational factor = presentValueOfPayments(rate, periods);
    // The residual value now; when it is too small to hold, it may only be left out where it is not the whole answer
    const std::optional<Rational> discounted = rate == zero ? std::optional<Rational>(one) : discounting(rate, periods).left;
    switch (chosen) {
    case lease::payment:
        if (!discounted && value == zero && residual != zero) {
            return std::nullopt;
        }
        return (value - (residual * discounted.value_or(zero))) / factor;
    case lease::value:
        if (!discounted && payment == zero && residual != zero) {
            return std::nullopt;
        }
        return (payment * factor) + (residual * discounted.value_or(zero));
    case lease::residual:
        // Beyond what the number library holds when what is left is nothing
        if (!discounted) {
            return std::nullopt;
        }
        return (value - (payment * factor)) / *discounted;
    default:
        return std::nullopt;
    }
}

// Fuel economy in miles per gallon: distance / fuel used (part C line 3)
std::optional<Rational> calculateMilesPerGallon(std::size_t chosen, const std::vector<Rational>& values)
{
    const Rational& distance = values.at(fuel::distance);
    const Rational& used = values.at(fuel::used);
    const Rational& economy = values.at(fuel::economy);
    switch (chosen) {
    case fuel::economy:
        return used == zero ? std::nullopt : std::optional<Rational>(distance / used);
    case fuel::distance:
        return used * economy;
    case fuel::used:
        return economy == zero ? std::nullopt : std::optional<Rational>(distance / economy);
    default:
        return std::nullopt;
    }
}

// Fuel economy in litres per 100 km: fuel used / distance * 100 (part C line 3)
std::optional<Rational> calculateLitresPer100Kilometres(std::size_t chosen, const std::vector<Rational>& values)
{
    const Rational& distance = values.at(fuel::distance);
    const Rational& used = values.at(fuel::used);
    const Rational& economy = values.at(fuel::economy);
    const Rational hundred(100);
    switch (chosen) {
    case fuel::economy:
        return distance == zero ? std::nullopt : std::optional<Rational>(used * hundred / distance);
    case fuel::distance:
        return economy == zero ? std::nullopt : std::optional<Rational>(used * hundred / economy);
    case fuel::used:
        return distance * economy / hundred;
    default:
        return std::nullopt;
    }
}

}  // namespace

const std::array<WorksheetSpec, worksheetCount>& worksheets()
{
    // Windows 7's lists are sorted by name; the value calculated the first time is the payment or the fuel economy
    static const std::array<WorksheetSpec, worksheetCount> specs{
        WorksheetSpec{.name = "Mortgage",
                      .values = {"Purchase price", "Down payment", "Term (years)", "Interest rate (%)", "Monthly payment"},
                      .choices = {mortgage::down, mortgage::payment, mortgage::price, mortgage::term},
                      .firstChoice = mortgage::payment},
        WorksheetSpec{.name = "Vehicle lease",
                      .values = {"Lease value", "Lease period", "Payments per year", "Residual value", "Interest rate (%)",
                                 "Periodic payment"},
                      .choices = {lease::period, lease::value, lease::payment, lease::residual},
                      .firstChoice = lease::payment},
        WorksheetSpec{.name = "Fuel economy (mpg)",
                      .values = {"Distance (miles)", "Fuel used (gallons)", "Fuel economy (mpg)"},
                      .choices = {fuel::distance, fuel::economy, fuel::used},
                      .firstChoice = fuel::economy},
        WorksheetSpec{.name = "Fuel economy (L/100 km)",
                      .values = {"Distance (kilometers)", "Fuel used (liters)", "Fuel economy (L/100 km)"},
                      .choices = {fuel::distance, fuel::economy, fuel::used},
                      .firstChoice = fuel::economy},
    };
    return specs;
}

const WorksheetSpec& worksheetSpec(Worksheet worksheet)
{
    return worksheets().at(static_cast<std::size_t>(worksheet));
}

std::optional<Worksheet> worksheetOf(SidePanel panel)
{
    switch (panel) {
    case SidePanel::Mortgage:
        return Worksheet::Mortgage;
    case SidePanel::VehicleLease:
        return Worksheet::VehicleLease;
    case SidePanel::FuelEconomyMpg:
        return Worksheet::FuelEconomyMpg;
    case SidePanel::FuelEconomyLitres:
        return Worksheet::FuelEconomyLitres;
    case SidePanel::Basic:
    case SidePanel::UnitConversion:
    case SidePanel::DateCalculation:
        break;
    }
    return std::nullopt;
}

std::optional<Rational> calculateWorksheet(Worksheet worksheet, std::size_t chosen, const std::vector<Rational>& values)
{
    if (values.size() != worksheetSpec(worksheet).values.size() || chosen >= values.size()) {
        return std::nullopt;
    }
    // The number library reports what it cannot compute (a division by zero, an overflow) by throwing its error code
    try {
        switch (worksheet) {
        case Worksheet::Mortgage:
            return calculateMortgage(chosen, values);
        case Worksheet::VehicleLease:
            return calculateLease(chosen, values);
        case Worksheet::FuelEconomyMpg:
            return calculateMilesPerGallon(chosen, values);
        case Worksheet::FuelEconomyLitres:
            return calculateLitresPer100Kilometres(chosen, values);
        }
    } catch (const uint32_t /*error*/) {
        return std::nullopt;
    }
    return std::nullopt;
}

}  // namespace chrono_calc
