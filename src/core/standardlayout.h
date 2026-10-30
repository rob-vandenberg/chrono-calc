#pragma once

#include <span>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/standardlayout.h 0.1.6

// --- Version history ----------------------------------------------------
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

// One button of a view: the command it runs, its label (UTF-8) and where it sits
struct ButtonSpec
{
    std::string_view commandId;
    std::string_view label;
    Rectangle rectangle;
};

// The Standard view of the Windows 7 Calculator, measured on the real one (tests/reference/windows7-standard.md, part A)
namespace standard {

inline constexpr int clientWidth = 264;
inline constexpr int clientHeight = 331;
inline constexpr Rectangle displayFrame{.left = 13, .top = 13, .width = 238, .height = 63};
inline constexpr Rectangle expressionLine{.left = 17, .top = 16, .width = 230, .height = 18};
inline constexpr Rectangle memoryMark{.left = 13, .top = 44, .width = 17, .height = 23};
inline constexpr Rectangle mainNumber{.left = 35, .top = 33, .width = 211, .height = 36};

// The command id of the decimal-point button; the window shows the regional decimal separator as its label
inline constexpr std::string_view decimalCommand = "decimal";

// Every button of the Standard view, row by row from top left
std::span<const ButtonSpec> buttons();

}  // namespace standard

}  // namespace chrono_calc
