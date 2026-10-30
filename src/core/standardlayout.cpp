#include "core/standardlayout.h"

#include <array>
#include <cstddef>
#include <span>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/standardlayout.cpp 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: Initial: the Standard view's measures and buttons, as the Windows 7 Calculator (D-018)

namespace chrono_calc::standard {

namespace {

// Buttons are 43 x 33; the columns and rows where they start
constexpr int buttonWidth = 43;
constexpr int buttonHeight = 33;
constexpr std::array columns = {13, 62, 111, 160, 208};
constexpr std::array rows = {82, 122, 162, 203, 242, 282};

constexpr ButtonSpec at(std::string_view commandId, std::string_view label, std::size_t column, std::size_t row)
{
    return {.commandId = commandId,
            .label = label,
            .rectangle = {.left = columns.at(column), .top = rows.at(row), .width = buttonWidth, .height = buttonHeight}};
}

constexpr ButtonSpec wide(ButtonSpec spec, int width)
{
    spec.rectangle.width = width;
    return spec;
}

constexpr ButtonSpec tall(ButtonSpec spec, int height)
{
    spec.rectangle.height = height;
    return spec;
}

// The labels are our own text; the decimal point's label is replaced by the regional separator in the window
constexpr std::array standardButtons = {
    at("memory.clear", "MC", 0, 0),
    at("memory.recall", "MR", 1, 0),
    at("memory.store", "MS", 2, 0),
    at("memory.add", "M+", 3, 0),
    at("memory.subtract", "M-", 4, 0),
    at("backspace", "←", 0, 1),
    at("clear_entry", "CE", 1, 1),
    at("clear", "C", 2, 1),
    at("negate", "±", 3, 1),
    at("sqrt", "√", 4, 1),
    at("digit.7", "7", 0, 2),
    at("digit.8", "8", 1, 2),
    at("digit.9", "9", 2, 2),
    at("divide", "/", 3, 2),
    at("percent", "%", 4, 2),
    at("digit.4", "4", 0, 3),
    at("digit.5", "5", 1, 3),
    at("digit.6", "6", 2, 3),
    at("multiply", "*", 3, 3),
    at("reciprocal", "1/x", 4, 3),
    at("digit.1", "1", 0, 4),
    at("digit.2", "2", 1, 4),
    at("digit.3", "3", 2, 4),
    at("subtract", "-", 3, 4),
    tall(at("equals", "=", 4, 4), 74),
    wide(at("digit.0", "0", 0, 5), 92),
    at(decimalCommand, ".", 2, 5),
    at("add", "+", 3, 5),
};

}  // namespace

std::span<const ButtonSpec> buttons()
{
    return standardButtons;
}

}  // namespace chrono_calc::standard
