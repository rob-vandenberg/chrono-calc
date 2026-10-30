#pragma once

#include "Header Files/Rational.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/unitconversion.h 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the unit types and units of the Windows 7 Unit conversion panel, converted on the engine's exact
//          fractions (D-010, D-025); the number boxes' typed text and its value, shared with the Worksheets (D-027)

namespace chrono_calc {

// One unit of a unit type. Its size is given in the type's own base unit as an exact number written out ("0.0254",
// "5/9", "1e-10"), from the unit's definition; never a floating-point number (D-010). A temperature also has an offset:
// the base value of its zero (Kelvin is the base: 0 Celsius is 273.15 Kelvin, 0 Fahrenheit 459.67 * 5/9).
struct UnitSpec
{
    std::string_view name;
    std::string_view size;
    std::string_view offset = "0";
};

// One unit type of the panel's first list (Windows 7: Angle, Area, Energy and so on) and its units in the order of the
// From and To lists (both lists are the same; choosing the type sets both to the first unit)
struct UnitTypeSpec
{
    std::string_view name;
    std::vector<UnitSpec> units;
};

// Every unit type of the Windows 7 Unit conversion panel, in its order (tests/reference/windows7-unitconversion.md)
const std::vector<UnitTypeSpec>& unitTypes();

// An exact number written out as UnitSpec writes it: digits with an optional decimal point, an optional exponent
// ("e-10") and an optional "/" with a divisor in the same form; a "-" first makes it negative. Nothing for any other text.
std::optional<CalcEngine::Rational> exactNumber(std::string_view text);

// The longest text a number box of a side panel holds, and the most digits its number may have (Unit conversion and
// the Worksheets alike: windows7-unitconversion.md part D line 2, windows7-worksheets.md part B line 3)
inline constexpr std::size_t mostNumberCharacters = 25;
inline constexpr std::size_t mostNumberDigits = 16;

// The characters such a box takes as they are typed, at most mostNumberCharacters of them: digits, the decimal symbol,
// + - and a small e; every other character is left out
std::wstring acceptedNumberText(std::wstring_view text, wchar_t decimal);

// The number such a box holds, read as Windows 7 reads both panels' boxes (windows7-unitconversion.md part G,
// windows7-worksheets.md part E): a sign, digits with one decimal symbol, and an exponent (e, a sign, at most four
// digits), as the exact number it is written as; at most 16 digits, where the zeros before the whole part's first
// other digit do not count but every digit after the decimal symbol does. Without digits the number is 0 ("," "-")
// or, with an exponent, 1 ("e5" is 1e+5); an exponent without digits is 0 ("1e" is 1). Nothing for an empty box or any
// other text.
std::optional<CalcEngine::Rational> typedNumber(std::wstring_view text, wchar_t decimal);

// The digits a panel writes a result with (Windows 7: 16, as the Standard view)
inline constexpr int32_t writtenDigits = 16;

// A panel's result as Windows 7 writes it: 16 digits, the decimal symbol, no grouping; with an exponent ("1,e+7",
// "1,428571428571429e-2") when the number was typed with one, otherwise the engine's own writing, which takes an
// exponent only when the number does not fit ("6,21371192237334e+16"). The number library must be in decimal.
std::wstring writtenNumber(const CalcEngine::Rational& value, bool exponentForm, wchar_t decimal);

// The value in the unit "from" converted to the unit "to" of the same type: through the base unit, with the engine's
// exact fractions only. Nothing when the number library cannot hold the result (it throws then).
std::optional<CalcEngine::Rational> convertUnit(const CalcEngine::Rational& value, const UnitSpec& from, const UnitSpec& to);

}  // namespace chrono_calc
