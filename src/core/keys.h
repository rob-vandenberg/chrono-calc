#pragma once

#include "core/view.h"

#include <cstdint>
#include <optional>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/keys.h 0.2.7

// --- Version history ----------------------------------------------------
// v0.2.7: The keys of the Scientific view: letters, ( ) # ! ;, F3 to F5, Ctrl+Y/G/B/S/O/T
// v0.1.6: The keys without a character (Escape, Delete, Backspace, F9) and the Ctrl keys of the memory
// v0.0.4: Initial: the typed characters of the Standard view and their commands

namespace chrono_calc {

// The command a typed character gives in a view, as in Windows 7. Both views: digits, . and , for the decimal point,
// + - * / and = (also Enter), @ for the square root, r for the reciprocal. Standard: % for percent. Scientific: ( ) # (x³)
// ! (n!) ; (Int), and the letters i (Inv) s o t (sin cos tan) n (ln) l (log) m (dms) p (pi) v (F-E) x and e (Exp)
// y (x^y) q (x²) d (Mod); letters typed with Shift do nothing there.
std::optional<std::string_view> commandForCharacter(char32_t character, View view = View::Standard);

// The keys of the Standard view that type no character
enum class Key : std::uint8_t
{
    Escape,
    Delete,
    Backspace,
    Enter,
    F3,
    F4,
    F5,
    F9,
};

// The command such a key gives, as in Windows 7: Escape = C, Delete = CE, Backspace, Enter = equals, F9 = plus/minus;
// in the Scientific view F3, F4 and F5 = Degrees, Radians and Grads
std::optional<std::string_view> commandForKey(Key key, View view = View::Standard);

// The command of Ctrl with this letter, as in Windows 7: L = MC, R = MR, M = MS, P = M+, Q = M-; in the Scientific view
// also Y = y√x, G = 10^x, B = ∛x, S = sinh, O = cosh, T = tanh
std::optional<std::string_view> commandForControlLetter(char32_t letter, View view = View::Standard);

}  // namespace chrono_calc
