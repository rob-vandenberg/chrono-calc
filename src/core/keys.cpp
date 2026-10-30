#include "core/keys.h"

#include <optional>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/keys.cpp 0.0.4

// --- Version history ----------------------------------------------------
// v0.0.4: Initial: the typed characters of the Standard view and their commands

namespace chrono_calc {

std::optional<std::string_view> commandForCharacter(char32_t character)
{
    switch (character) {
    case U'0': return "digit.0";
    case U'1': return "digit.1";
    case U'2': return "digit.2";
    case U'3': return "digit.3";
    case U'4': return "digit.4";
    case U'5': return "digit.5";
    case U'6': return "digit.6";
    case U'7': return "digit.7";
    case U'8': return "digit.8";
    case U'9': return "digit.9";
    case U'.':
    case U',': return "decimal";
    case U'+': return "add";
    case U'-': return "subtract";
    case U'*': return "multiply";
    case U'/': return "divide";
    case U'=':
    case U'\r':
    case U'\n': return "equals";
    case U'@': return "sqrt";
    case U'%': return "percent";
    case U'r':
    case U'R': return "reciprocal";
    default: return std::nullopt;
    }
}

}  // namespace chrono_calc
