#pragma once

#include <optional>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/keys.h 0.0.4

// --- Version history ----------------------------------------------------
// v0.0.4: Initial: the typed characters of the Standard view and their commands

namespace chrono_calc {

// The command a typed character gives in the Standard view, as in Windows 7: digits, . and , for the decimal point,
// + - * / and = (also Enter), @ for the square root, % for percent, r for the reciprocal. Keys without a character
// (Escape, Delete, Backspace, F9, Ctrl+M and the like) belong to the window (task 1.2).
std::optional<std::string_view> commandForCharacter(char32_t character);

}  // namespace chrono_calc
