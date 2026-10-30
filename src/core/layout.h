#pragma once

#include "core/view.h"

#include <cstdint>
#include <span>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/layout.h 0.2.7

// --- Version history ----------------------------------------------------
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
// flat. A toggle (Inv, F-E) has the function face, amber while it is on.
enum class ButtonFace : std::uint8_t
{
    Digit,
    Function,
    Memory,
    Toggle,
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

// One of the Scientific view's angle units (a round choice button)
struct AngleSpec
{
    std::string_view commandId;
    std::string_view label;
    AngleUnit unit = AngleUnit::Degrees;
    Rectangle rectangle;
};

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
    // Scientific only (empty in Standard): the frame of the angle units, the units, the bracket counter "(=1"
    Rectangle angleFrame;
    std::span<const AngleSpec> angles;
    Rectangle bracketCounter;
};

const ViewLayout& layoutOf(View view);

// The command id of the decimal-point button; the window shows the regional decimal separator as its label
inline constexpr std::string_view decimalCommand = "decimal";

}  // namespace chrono_calc
