#include "core/keys.h"

#include "core/view.h"

#include <optional>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/keys.cpp 0.3.8

// --- Version history ----------------------------------------------------
// v0.3.8: The keys of the Programmer view: & | ^ ~ < > % ( ), a to f, j and k, F2 to F8 and F12
// v0.2.7: The keys of the Scientific view: letters, ( ) # ! ;, F3 to F5, Ctrl+Y/G/B/S/O/T
// v0.1.6: The keys without a character (Escape, Delete, Backspace, F9) and the Ctrl keys of the memory
// v0.0.4: Initial: the typed characters of the Standard view and their commands

namespace chrono_calc {

namespace {

// The characters both views share
std::optional<std::string_view> sharedCharacter(char32_t character)
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
    case U'r': return "reciprocal";
    default: return std::nullopt;
    }
}

std::optional<std::string_view> scientificCharacter(char32_t character)
{
    switch (character) {
    case U'(': return "open_bracket";
    case U')': return "close_bracket";
    case U'#': return "cube";
    case U'!': return "factorial";
    case U';': return "integer";
    case U'i': return "inverse";
    case U's': return "sin";
    case U'o': return "cos";
    case U't': return "tan";
    case U'n': return "ln";
    case U'l': return "log";
    case U'm': return "dms";
    case U'p': return "pi";
    case U'v': return "scientific_notation";
    case U'x':
    case U'e': return "exponent";
    case U'y': return "power";
    case U'q': return "square";
    case U'd': return "modulo";
    default: return std::nullopt;
    }
}

std::optional<std::string_view> programmerCharacter(char32_t character)
{
    switch (character) {
    case U'&': return "and";
    case U'|': return "or";
    case U'^': return "xor";
    case U'~': return "not";
    case U'<': return "shift_left";
    case U'>': return "shift_right";
    case U'%': return "modulo";
    case U'(': return "open_bracket";
    case U')': return "close_bracket";
    case U'a':
    case U'A': return "digit.a";
    case U'b':
    case U'B': return "digit.b";
    case U'c':
    case U'C': return "digit.c";
    case U'd':
    case U'D': return "digit.d";
    case U'e':
    case U'E': return "digit.e";
    case U'f':
    case U'F': return "digit.f";
    case U'j': return "rotate_left";
    case U'k': return "rotate_right";
    default: return std::nullopt;
    }
}

std::optional<std::string_view> programmerKey(Key key)
{
    switch (key) {
    case Key::F5: return "base.hexadecimal";
    case Key::F6: return "base.decimal";
    case Key::F7: return "base.octal";
    case Key::F8: return "base.binary";
    case Key::F12: return "word.qword";
    case Key::F2: return "word.dword";
    case Key::F3: return "word.word";
    case Key::F4: return "word.byte";
    default: return std::nullopt;
    }
}

std::optional<std::string_view> scientificKey(Key key)
{
    switch (key) {
    case Key::F3: return "angle.degrees";
    case Key::F4: return "angle.radians";
    case Key::F5: return "angle.grads";
    default: return std::nullopt;
    }
}

}  // namespace

std::optional<std::string_view> commandForCharacter(char32_t character, View view)
{
    if (const auto command = sharedCharacter(character)) {
        return command;
    }
    if (view == View::Scientific) {
        return scientificCharacter(character);
    }
    if (view == View::Programmer) {
        return programmerCharacter(character);
    }
    switch (character) {
    case U'%': return "percent";
    case U'R': return "reciprocal";
    default: return std::nullopt;
    }
}

std::optional<std::string_view> commandForKey(Key key, View view)
{
    switch (key) {
    case Key::Escape: return "clear";
    case Key::Delete: return "clear_entry";
    case Key::Backspace: return "backspace";
    case Key::Enter: return "equals";
    case Key::F9: return "negate";
    default: break;
    }
    switch (view) {
    case View::Scientific: return scientificKey(key);
    case View::Programmer: return programmerKey(key);
    case View::Standard: break;
    }
    return std::nullopt;
}

std::optional<std::string_view> commandForControlLetter(char32_t letter, View view)
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
    default: break;
    }
    if (view != View::Scientific) {
        return std::nullopt;
    }
    switch (letter) {
    case U'y':
    case U'Y': return "root";
    case U'g':
    case U'G': return "power_of_ten";
    case U'b':
    case U'B': return "cube_root";
    case U's':
    case U'S': return "sinh";
    case U'o':
    case U'O': return "cosh";
    case U't':
    case U'T': return "tanh";
    default: return std::nullopt;
    }
}

}  // namespace chrono_calc
