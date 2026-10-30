#pragma once

#include "core/view.h"

#include <cstdint>
#include <optional>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/keys.h 0.3.8

// --- Version history ----------------------------------------------------
// v0.3.8: The keys of the Programmer view: & | ^ ~ < > % ( ), a to f, j and k, F2 to F8 and F12
// v0.2.7: The keys of the Scientific view: letters, ( ) # ! ;, F3 to F5, Ctrl+Y/G/B/S/O/T
// v0.1.6: The keys without a character (Escape, Delete, Backspace, F9) and the Ctrl keys of the memory
// v0.0.4: Initial: the typed characters of the Standard view and their commands

namespace chrono_calc {

// The command a typed character gives in a view, as in Windows 7. Both views: digits, . and , for the decimal point,
// + - * / and = (also Enter), @ for the square root, r for the reciprocal. Standard: % for percent. Scientific: ( ) # (x³)
// ! (n!) ; (Int), and the letters i (Inv) s o t (sin cos tan) n (ln) l (log) m (dms) p (pi) v (F-E) x and e (Exp)
// y (x^y) q (x²) d (Mod); letters typed with Shift do nothing there. Programmer: & (And) | (Or) ^ (Xor) ~ (Not)
// < (Lsh) > (Rsh) % (Mod) ( ), a to f and A to F (the hexadecimal digits, also with Shift), j (RoL) k (RoR). The
// window gives a letter key pressed without Shift as its small letter, also with Caps Lock on, as Windows 7 reads keys.
std::optional<std::string_view> commandForCharacter(char32_t character, View view = View::Standard);

// The keys of the Standard view that type no character
enum class Key : std::uint8_t
{
    Escape,
    Delete,
    Backspace,
    Enter,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F12,
};

// The command such a key gives, as in Windows 7: Escape = C, Delete = CE, Backspace, Enter = equals, F9 = plus/minus;
// in the Scientific view F3, F4 and F5 = Degrees, Radians and Grads; in the Programmer view F5 to F8 = Hex, Dec, Oct,
// Bin and F12, F2, F3, F4 = Qword, Dword, Word, Byte
std::optional<std::string_view> commandForKey(Key key, View view = View::Standard);

// The command of Ctrl with this letter, as in Windows 7: L = MC, R = MR, M = MS, P = M+, Q = M-; in the Scientific view
// also Y = y√x, G = 10^x, B = ∛x, S = sinh, O = cosh, T = tanh
std::optional<std::string_view> commandForControlLetter(char32_t letter, View view = View::Standard);

}  // namespace chrono_calc
