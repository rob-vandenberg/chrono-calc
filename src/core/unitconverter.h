#pragma once

#include "core/unitconversion.h"

#include "Header Files/Rational.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/unitconverter.h 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Unit conversion panel's state: the type, the two units, the From text and the To result (D-025)

namespace chrono_calc {

// The Unit conversion panel (tests/reference/windows7-unitconversion.md): the chosen type, the From and To units, the
// text of the From box and the result the To box shows. The From box is its own; the calculator's display never
// changes it, and it never goes into the display (Windows 7).
class UnitConverter
{
public:
    // The regional decimal symbol, which the From box takes and the To box writes
    explicit UnitConverter(wchar_t decimal);

    // Choosing a type sets both units to its first one and empties the From box (Windows 7); an index beyond the lists
    // changes nothing
    void chooseType(std::size_t type);
    void chooseFrom(std::size_t unit);
    void chooseTo(std::size_t unit);
    // The text typed into the From box: what the box takes of it (part D line 2)
    void setInput(std::wstring_view text);

    [[nodiscard]] std::size_t type() const;
    [[nodiscard]] std::size_t from() const;
    [[nodiscard]] std::size_t to() const;
    [[nodiscard]] const std::wstring& input() const;
    // The regional decimal symbol the boxes use
    [[nodiscard]] wchar_t decimal() const;
    // The value of the From box, when it is a number the panel converts (at most 16 digits); nothing otherwise
    [[nodiscard]] std::optional<CalcEngine::Rational> inputValue() const;
    // The To box: the From value converted, as the engine writes a number with 16 digits; empty while From is empty or
    // not a number. Calculated by the caller (Calculator), which sets the number library up for it first.
    [[nodiscard]] const std::wstring& result() const;
    void setResult(std::wstring result);

    // The characters the From box takes as they are typed, at most 25 of them: digits, the decimal symbol, + - and a
    // small e; every other character is left out
    [[nodiscard]] std::wstring accepted(std::wstring_view text) const;

    // The longest text the From box holds, and the most digits a number in it may have
    static constexpr std::size_t mostCharacters = mostNumberCharacters;
    static constexpr std::size_t mostDigits = mostNumberDigits;

private:
    wchar_t m_decimal;
    std::size_t m_type = 0;
    std::size_t m_from = 0;
    std::size_t m_to = 0;
    std::wstring m_input;
    std::wstring m_result;
};

}  // namespace chrono_calc
