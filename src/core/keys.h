#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/keys.h 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: The keys without a character (Escape, Delete, Backspace, F9) and the Ctrl keys of the memory
// v0.0.4: Initial: the typed characters of the Standard view and their commands

namespace chrono_calc {

// The command a typed character gives in the Standard view, as in Windows 7: digits, . and , for the decimal point,
// + - * / and = (also Enter), @ for the square root, % for percent, r for the reciprocal.
std::optional<std::string_view> commandForCharacter(char32_t character);

// The keys of the Standard view that type no character
enum class Key : std::uint8_t
{
    Escape,
    Delete,
    Backspace,
    Enter,
    F9,
};

// The command such a key gives, as in Windows 7: Escape = C, Delete = CE, Backspace, Enter = equals, F9 = plus/minus
std::optional<std::string_view> commandForKey(Key key);

// The command of Ctrl with this letter, as in Windows 7: L = MC, R = MR, M = MS, P = M+, Q = M-
std::optional<std::string_view> commandForControlLetter(char32_t letter);

}  // namespace chrono_calc
