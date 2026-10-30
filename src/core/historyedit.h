#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/historyedit.h 0.5.12

// --- Version history ----------------------------------------------------
// v0.5.12: Initial: reads an edited history entry and a typed number (Edit > History > Edit, the Statistics edit, D-024)

namespace chrono_calc {

// A number as it was typed: its digits with the decimal point where it was, and an exponent ("1,e+5")
struct TypedNumber
{
    bool negative = false;
    // The digits; '.' marks the decimal point
    std::wstring mantissa;
    bool hasExponent = false;
    bool exponentNegative = false;
    std::wstring exponent;
};

// One part of an edited calculation, in the order the engine takes them
struct EditStep
{
    enum class Kind : std::uint8_t
    {
        Number,
        // A binary operator (+ - * / ^ Mod yroot)
        Operator,
        // A function: its name and an opening bracket ("sqrt("); its commands come at the matching Close
        Function,
        Open,
        Close,
    };
    Kind kind = Kind::Number;
    TypedNumber number;
    // The engine command of an operator; the commands of a function (Inv first where it is an inverse form, an angle
    // unit first where it is one of sin, cos, tan)
    std::vector<std::uintptr_t> commands;
    // How the entry writes it, as the engine does: "+", "Mod", "sqrt"
    std::wstring text;
};

// The regional separators and how many digits a number may have (16 in Standard, 32 in Scientific)
struct NumberRules
{
    wchar_t decimal = L'.';
    wchar_t thousands = L',';
    std::size_t mostDigits = 16;
};

// Reads an edited history entry as Windows 7 does (tests/reference/windows7-history.md part D): numbers (the regional
// decimal symbol; the thousands symbol is skipped; a minus right before a number at the start, after a bracket or an
// operator is its sign; "1,e+5"), the operators + - * / ^ Mod yroot, brackets, and the functions by the names the
// engine writes them with, followed by their bracket ("sqrt(9)", "sind(30)", "negate(2)"); spaces anywhere between.
// Nothing for a text that is empty, unbalanced, has an unknown word or a number with too many digits.
std::optional<std::vector<EditStep>> readEdit(std::wstring_view text, const NumberRules& rules);

// Reads one number, as readEdit reads it ("-1,5", "1,e+5"), with spaces around it; nothing for anything else
std::optional<TypedNumber> readNumber(std::wstring_view text, const NumberRules& rules);

}  // namespace chrono_calc
