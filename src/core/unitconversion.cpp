#include "core/unitconversion.h"

#include "Header Files/Rational.h"
#include "Header Files/RationalMath.h"
#include "Ratpack/ratpak.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/unitconversion.cpp 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the unit types and units of the Windows 7 Unit conversion panel, converted on the engine's exact
//          fractions (D-010, D-025); the number boxes' typed text and its value, shared with the Worksheets (D-027)

namespace chrono_calc {

using CalcEngine::Rational;

namespace {

// A run of digits as an exact whole number; the count of digits read goes to "length"
Rational digitsOf(std::string_view text, std::size_t& length)
{
    Rational number(0);
    length = 0;
    while (length < text.size() && text[length] >= '0' && text[length] <= '9') {
        number = (number * Rational(10)) + Rational(static_cast<uint32_t>(text[length] - '0'));
        ++length;
    }
    return number;
}

bool isDigit(wchar_t character)
{
    return character >= L'0' && character <= L'9';
}

// The exponent of a typed number as exactNumber reads it ("e-12"; nothing for "1e", whose exponent is 0), from the
// text after the e: a sign and at most four digits, as the engine's own exponent input (CalcInput); nothing when it is
// not that
std::optional<std::string> exponentText(std::wstring_view exponent)
{
    constexpr std::size_t mostExponentDigits = 4;
    std::string sign;
    if (exponent.starts_with(L'+') || exponent.starts_with(L'-')) {
        sign = exponent.starts_with(L'-') ? "-" : "";
        exponent.remove_prefix(1);
    }
    if (exponent.size() > mostExponentDigits || !std::ranges::all_of(exponent, isDigit)) {
        return std::nullopt;
    }
    if (exponent.empty()) {
        return std::string();
    }
    std::string written = "e" + sign;
    for (const wchar_t character : exponent) {
        written += static_cast<char>(character);
    }
    return written;
}

// 10 to the power of the exponent, exact, by squaring
Rational powerOfTen(int32_t exponent)
{
    Rational power(1);
    Rational square(10);
    for (int32_t remaining = exponent < 0 ? -exponent : exponent; remaining > 0; remaining /= 2) {
        if (remaining % 2 == 1) {
            power *= square;
        }
        square *= square;
    }
    return exponent < 0 ? Rational(1) / power : power;
}

// One number without "/": "-12.5e-3", or "pi" (the engine's own pi, for the angles)
std::optional<Rational> decimalNumber(std::string_view text)
{
    if (text == "pi") {
        return CalcEngine::RationalMath::ACos(Rational(-1), AngleType::Radians);
    }
    const bool negative = text.starts_with('-');
    if (negative) {
        text.remove_prefix(1);
    }
    std::size_t length = 0;
    Rational number = digitsOf(text, length);
    bool anyDigit = length > 0;
    text.remove_prefix(length);
    int32_t exponent = 0;
    if (text.starts_with('.')) {
        text.remove_prefix(1);
        const Rational fraction = digitsOf(text, length);
        number = (number * powerOfTen(static_cast<int32_t>(length))) + fraction;
        exponent = -static_cast<int32_t>(length);
        anyDigit = anyDigit || length > 0;
        text.remove_prefix(length);
    }
    if (!anyDigit) {
        return std::nullopt;
    }
    if (text.starts_with('e')) {
        text.remove_prefix(1);
        const bool negativeExponent = text.starts_with('-');
        if (negativeExponent || text.starts_with('+')) {
            text.remove_prefix(1);
        }
        // Four digits at most, as the engine's own exponent input (CalcInput)
        constexpr std::size_t mostExponentDigits = 4;
        std::size_t exponentLength = 0;
        int32_t written = 0;
        while (exponentLength < text.size() && exponentLength < mostExponentDigits && text[exponentLength] >= '0'
               && text[exponentLength] <= '9') {
            written = (written * 10) + (text[exponentLength] - '0');
            ++exponentLength;
        }
        if (exponentLength == 0) {
            return std::nullopt;
        }
        exponent += negativeExponent ? -written : written;
        text.remove_prefix(exponentLength);
    }
    if (!text.empty()) {
        return std::nullopt;
    }
    number *= powerOfTen(exponent);
    return negative ? -number : number;
}

// The value in the type's base unit
Rational toBase(const Rational& value, const Rational& size, const Rational& offset)
{
    return (value * size) + offset;
}

}  // namespace

// The unit types of Windows 7 in its order, each unit's size in the type's base unit (tests/reference/
// windows7-unitconversion.md part B). The sizes are the units' exact definitions (the international yard and pound of
// 1959, the US and imperial gallons, the IT calorie, standard gravity 9.80665 m/s² for the foot-pound). Windows 7's own
// values where a unit has no exact definition: Mach at 340.2933 m/s, the printer's pica at 0.166044 inch. Not copied,
// because Windows 7's rounded values give wrong results (D-025): the millimetre of mercury (Windows 7: 133.3 Pa, so 1
// atmosphere showed 760,1275318829707 instead of 760), the pound per square inch (6894.757 Pa instead of 0.45359237 *
// 9.80665 / 0.0254²), the electronvolt (the CODATA 2002 value instead of the exact SI one) and the IT BTU (1055.056 J
// instead of 4.1868 J/g * 453.59237 g * 5/9 = 1055.05585262 J).
const std::vector<UnitTypeSpec>& unitTypes()
{
    // The foot-pound in joules: 0.3048 m * 0.45359237 kg * 9.80665 m/s²
    static constexpr std::string_view footPound = "1.3558179483314004";
    static const std::vector<UnitTypeSpec> types = {
        {.name = "Angle", .units = {{.name = "Degree", .size = "1"}, {.name = "Gradian", .size = "0.9"},
                                    {.name = "Radian", .size = "180/pi"}}},
        {.name = "Area",
         .units = {{.name = "Acres", .size = "4046.8564224"}, {.name = "Hectares", .size = "10000"},
                   {.name = "Square centimeter", .size = "0.0001"}, {.name = "Square feet", .size = "0.09290304"},
                   {.name = "Square inch", .size = "0.00064516"}, {.name = "Square kilometer", .size = "1000000"},
                   {.name = "Square meters", .size = "1"}, {.name = "Square mile", .size = "2589988.110336"},
                   {.name = "Square millimeter", .size = "0.000001"}, {.name = "Square Yard", .size = "0.83612736"}}},
        {.name = "Energy",
         .units = {{.name = "British Thermal Unit", .size = "1055.05585262"}, {.name = "Calorie", .size = "4.1868"},
                   {.name = "Electron-Volts", .size = "1.602176634e-19"}, {.name = "Foot-Pound", .size = footPound},
                   {.name = "Joule", .size = "1"}, {.name = "Kilocalorie", .size = "4186.8"},
                   {.name = "Kilojoule", .size = "1000"}}},
        {.name = "Length",
         .units = {{.name = "Angstrom", .size = "1e-10"}, {.name = "Centimeters", .size = "0.01"},
                   {.name = "Chain", .size = "20.1168"}, {.name = "Fathom", .size = "1.8288"},
                   {.name = "Feet", .size = "0.3048"}, {.name = "Hand", .size = "0.1016"},
                   {.name = "Inch", .size = "0.0254"}, {.name = "Kilometers", .size = "1000"},
                   {.name = "Link", .size = "0.201168"}, {.name = "Meter", .size = "1"},
                   {.name = "Microns", .size = "0.000001"}, {.name = "Mile", .size = "1609.344"},
                   {.name = "Millimeters", .size = "0.001"}, {.name = "Nanometer", .size = "1e-9"},
                   {.name = "Nautical Miles", .size = "1852"}, {.name = "PICA", .size = "0.0042175176"},
                   {.name = "Rods", .size = "5.0292"}, {.name = "Span", .size = "0.2286"},
                   {.name = "Yard", .size = "0.9144"}}},
        {.name = "Power",
         .units = {{.name = "BTU/minute", .size = "1055.05585262/60"}, {.name = "Foot-Pound/minute", .size = "1.3558179483314004/60"},
                   {.name = "Horsepower", .size = "745.69987158227022"}, {.name = "Kilowatt", .size = "1000"},
                   {.name = "Watt", .size = "1"}}},
        {.name = "Pressure",
         .units = {{.name = "Atmosphere", .size = "101325"}, {.name = "Bar", .size = "100000"},
                   {.name = "Kilo Pascal", .size = "1000"}, {.name = "Millimeter of mercury", .size = "101325/760"},
                   {.name = "Pascal", .size = "1"},
                   {.name = "Pound per square inch (PSI)", .size = "4.4482216152605/0.00064516"}}},
        {.name = "Temperature",
         .units = {{.name = "Degrees Celsius", .size = "1", .offset = "273.15"},
                   {.name = "Degrees Fahrenheit", .size = "5/9", .offset = "2298.35/9"},
                   {.name = "Kelvin", .size = "1"}}},
        {.name = "Time",
         .units = {{.name = "Day", .size = "86400"}, {.name = "Hour", .size = "3600"},
                   {.name = "Microsecond", .size = "0.000001"}, {.name = "Millisecond", .size = "0.001"},
                   {.name = "Minute", .size = "60"}, {.name = "Second", .size = "1"}, {.name = "Week", .size = "604800"}}},
        {.name = "Velocity",
         .units = {{.name = "Centimeter per second", .size = "0.01"}, {.name = "Feet per second", .size = "0.3048"},
                   {.name = "Kilometer per hour", .size = "1/3.6"}, {.name = "Knots", .size = "1852/3600"},
                   {.name = "Mach (at std. atm)", .size = "340.2933"}, {.name = "Meter per second", .size = "1"},
                   {.name = "Miles per hour", .size = "0.44704"}}},
        {.name = "Volume",
         .units = {{.name = "Cubic centimeter", .size = "0.000001"}, {.name = "Cubic feet", .size = "0.028316846592"},
                   {.name = "Cubic inch", .size = "0.000016387064"}, {.name = "Cubic meter", .size = "1"},
                   {.name = "Cubic yard", .size = "0.764554857984"}, {.name = "Fluid ounce (UK)", .size = "0.0000284130625"},
                   {.name = "Fluid ounce (US)", .size = "0.0000295735295625"}, {.name = "Gallon (UK)", .size = "0.00454609"},
                   {.name = "Gallon (US)", .size = "0.003785411784"}, {.name = "Liter", .size = "0.001"},
                   {.name = "Pint (UK)", .size = "0.00056826125"}, {.name = "Pint (US)", .size = "0.000473176473"},
                   {.name = "Quart (UK)", .size = "0.0011365225"}, {.name = "Quart (US)", .size = "0.000946352946"}}},
        {.name = "Weight/Mass",
         .units = {{.name = "Carat", .size = "0.0002"}, {.name = "Centigram", .size = "0.00001"},
                   {.name = "Decigram", .size = "0.0001"}, {.name = "Dekagram", .size = "0.01"},
                   {.name = "Gram", .size = "0.001"}, {.name = "Hectogram", .size = "0.1"},
                   {.name = "Kilogram", .size = "1"}, {.name = "Long ton", .size = "1016.0469088"},
                   {.name = "Milligram", .size = "0.000001"}, {.name = "Ounce", .size = "0.028349523125"},
                   {.name = "Pound", .size = "0.45359237"}, {.name = "Short ton", .size = "907.18474"},
                   {.name = "Stone", .size = "6.35029318"}, {.name = "Tonne", .size = "1000"}}},
    };
    return types;
}

std::optional<Rational> exactNumber(std::string_view text)
{
    const std::size_t slash = text.find('/');
    if (slash == std::string_view::npos) {
        return decimalNumber(text);
    }
    const std::optional<Rational> dividend = decimalNumber(text.substr(0, slash));
    const std::optional<Rational> divisor = decimalNumber(text.substr(slash + 1));
    if (!dividend || !divisor || *divisor == Rational(0)) {
        return std::nullopt;
    }
    return *dividend / *divisor;
}

std::optional<Rational> convertUnit(const Rational& value, const UnitSpec& from, const UnitSpec& to)
{
    const std::optional<Rational> fromSize = exactNumber(from.size);
    const std::optional<Rational> fromOffset = exactNumber(from.offset);
    const std::optional<Rational> toSize = exactNumber(to.size);
    const std::optional<Rational> toOffset = exactNumber(to.offset);
    if (!fromSize || !fromOffset || !toSize || !toOffset || *toSize == Rational(0)) {
        return std::nullopt;
    }
    // The number library reports what it cannot compute (an overflow) by throwing its error code
    try {
        return (toBase(value, *fromSize, *fromOffset) - *toOffset) / *toSize;
    } catch (const uint32_t /*error*/) {
        return std::nullopt;
    }
}

std::wstring acceptedNumberText(std::wstring_view text, wchar_t decimal)
{
    std::wstring taken;
    for (const wchar_t character : text) {
        if (taken.size() == mostNumberCharacters) {
            break;
        }
        if (isDigit(character) || character == decimal || character == L'+' || character == L'-' || character == L'e') {
            taken += character;
        }
    }
    return taken;
}

std::optional<Rational> typedNumber(std::wstring_view text, wchar_t decimal)
{
    if (text.empty()) {
        return std::nullopt;
    }
    const bool negative = text.starts_with(L'-');
    if (negative || text.starts_with(L'+')) {
        text.remove_prefix(1);
    }
    const std::size_t exponentAt = text.find(L'e');
    std::string written = negative ? "-" : "";
    std::size_t digits = 0;
    bool decimalSeen = false;
    bool anyDigit = false;
    for (const wchar_t character : text.substr(0, exponentAt)) {
        if (isDigit(character)) {
            // Zeros before the whole part's first other digit do not count
            if (decimalSeen || character != L'0' || digits > 0) {
                ++digits;
            }
            anyDigit = true;
            written += static_cast<char>(character);
        } else if (character == decimal && !decimalSeen) {
            decimalSeen = true;
            written += '.';
        } else {
            return std::nullopt;
        }
    }
    if (digits > mostNumberDigits) {
        return std::nullopt;
    }
    if (!anyDigit) {
        written += exponentAt == std::wstring_view::npos ? "0" : "1";
    }
    if (exponentAt != std::wstring_view::npos) {
        const std::optional<std::string> exponent = exponentText(text.substr(exponentAt + 1));
        if (!exponent) {
            return std::nullopt;
        }
        written += *exponent;
    }
    return exactNumber(written);
}

std::wstring writtenNumber(const Rational& value, bool exponentForm, wchar_t decimal)
{
    if (!exponentForm || value == Rational(0)) {
        return value.ToString(10, NumberFormat::Float, writtenDigits);
    }
    // The engine's own exponent form keeps fewer digits for a number below 1 (it counts the zeros after the decimal
    // symbol), so the mantissa is written here: the number brought between 1 and 10, then its exponent
    const Rational size = CalcEngine::RationalMath::Abs(value);
    // The engine's exponent is right to within one (its mantissa may be cut, "0.e-21"); the loops below settle it
    const std::wstring rough = size.ToString(10, NumberFormat::Scientific, writtenDigits);
    const std::size_t exponentAt = rough.find(L'e');
    int64_t exponent = 0;
    if (exponentAt != std::wstring::npos) {
        for (const wchar_t character : std::wstring_view(rough).substr(exponentAt + 1)) {
            if (isDigit(character)) {
                exponent = (exponent * 10) + (character - L'0');
            }
        }
        exponent = rough.find(L'-', exponentAt) == std::wstring::npos ? exponent : -exponent;
    }
    // 10 to a whole power, in steps the engine's power takes (it refuses 10^x beyond e^100000, about 10^43429)
    const auto power = [](int64_t of) {
        constexpr int64_t step = 40000;
        Rational result(1);
        for (int64_t left = of; left != 0;) {
            const int64_t part = std::clamp(left, -step, step);
            result *= CalcEngine::RationalMath::Pow(Rational(10), Rational(static_cast<int32_t>(part)));
            left -= part;
        }
        return result;
    };
    while (size / power(exponent) >= Rational(10)) {
        ++exponent;
    }
    while (size / power(exponent) < Rational(1)) {
        --exponent;
    }
    std::wstring mantissa = (value / power(exponent)).ToString(10, NumberFormat::Float, writtenDigits);
    // 9,9999999999999999 is written as 10
    if (mantissa == L"10" || mantissa == L"-10") {
        ++exponent;
        mantissa = (value / power(exponent)).ToString(10, NumberFormat::Float, writtenDigits);
    }
    if (mantissa.find(decimal) == std::wstring::npos) {
        mantissa += decimal;
    }
    return mantissa + L'e' + (exponent < 0 ? L'-' : L'+') + std::to_wstring(exponent < 0 ? -exponent : exponent);
}

}  // namespace chrono_calc
