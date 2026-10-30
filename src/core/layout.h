#pragma once

#include "core/view.h"

#include <cstdint>
#include <span>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/layout.h 0.3.8

// --- Version history ----------------------------------------------------
// v0.3.8: The Programmer view: its buttons, the bases and word sizes, the bit display; choices of any kind
// v0.2.7: Every view's layout (Standard and Scientific); each button's face and its label under Inv (D-020)
// v0.1.6: Initial: the Standard view's measures and buttons, as the Windows 7 Calculator (D-018)

namespace chrono_calc {

// A rectangle in the window's client area, in pixels: left, top, width, height
struct Rectangle
{
    int left = 0;
    int top = 0;
    int width = 0;
    int height = 0;
};

// The face a button is drawn with, as Windows 7: digits light, functions and operators a darker blue, the memory row
// flat. A toggle (Inv, F-E) has the function face, amber while it is on. The hexadecimal digits A to F are flat.
enum class ButtonFace : std::uint8_t
{
    Digit,
    Function,
    Memory,
    Toggle,
    Flat,
};

// One button of a view: the command it runs, its label (UTF-8), its label while Inv is on (empty: the same), where it
// sits, its face, and how high a digit of its label's font is in pixels (Windows 7: 14 for the digits, 9 for the text
// labels; = and the backspace arrow are drawn in a large font, 1/x in a slightly larger one)
struct ButtonSpec
{
    std::string_view commandId;
    std::string_view label;
    std::string_view inverseLabel;
    Rectangle rectangle;
    ButtonFace face = ButtonFace::Function;
    double labelInkHeight = 9.0;
};

// A round choice button (an angle unit, a number base, a word size); the calculator says which is chosen
struct ChoiceSpec
{
    std::string_view commandId;
    std::string_view label;
    Rectangle rectangle;
};

// A bit position under the bit display ("63", "47" and so on), right-aligned or left-aligned in its place
struct BitLabelSpec
{
    std::string_view text;
    Rectangle rectangle;
    bool alignRight = false;
};

// The Programmer view's bit display: 64 bits in two rows of eight groups of four, bit 63 first
struct BitDisplaySpec
{
    Rectangle frame;
    std::span<const BitLabelSpec> labels;
};

// Where bit 0 to 63 of the bit display sits (Windows 7, windows7-programmer.md part A)
Rectangle bitRectangle(int bit);

// The layout of a view, measured on the Windows 7 Calculator (tests/reference/windows7-standard.md and
// windows7-scientific.md, part A)
struct ViewLayout
{
    int clientWidth = 0;
    int clientHeight = 0;
    Rectangle displayFrame;
    Rectangle expressionLine;
    Rectangle memoryMark;
    Rectangle mainNumber;
    std::span<const ButtonSpec> buttons;
    // Scientific and Programmer (empty in Standard): the flat frames around the choices, the choices (angle units;
    // bases and word sizes), the bracket counter "(=1"
    std::span<const Rectangle> frames;
    std::span<const ChoiceSpec> choices;
    Rectangle bracketCounter;
    // Programmer only (an empty frame elsewhere): the bit display
    BitDisplaySpec bitDisplay;
};

const ViewLayout& layoutOf(View view);

// The command id of the decimal-point button; the window shows the regional decimal separator as its label
inline constexpr std::string_view decimalCommand = "decimal";

}  // namespace chrono_calc
