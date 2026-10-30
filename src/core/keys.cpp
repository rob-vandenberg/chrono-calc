#include "core/keys.h"

#include <optional>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/keys.cpp 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: The keys without a character (Escape, Delete, Backspace, F9) and the Ctrl keys of the memory
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

std::optional<std::string_view> commandForKey(Key key)
{
    switch (key) {
    case Key::Escape: return "clear";
    case Key::Delete: return "clear_entry";
    case Key::Backspace: return "backspace";
    case Key::Enter: return "equals";
    case Key::F9: return "negate";
    }
    return std::nullopt;
}

std::optional<std::string_view> commandForControlLetter(char32_t letter)
{
    switch (letter) {
    case U'l':
    case U'L': return "memory.clear";
    case U'r':
    case U'R': return "memory.recall";
    case U'm':
    case U'M': return "memory.store";
    case U'p':
    case U'P': return "memory.add";
    case U'q':
    case U'Q': return "memory.subtract";
    default: return std::nullopt;
    }
}

}  // namespace chrono_calc
