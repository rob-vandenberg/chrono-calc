#include "core/layout.h"

#include "core/view.h"

#include <array>
#include <cstddef>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/layout.cpp 0.2.7

// --- Version history ----------------------------------------------------
// v0.2.7: Every view's layout (Standard and Scientific); each button's face and its label under Inv (D-020)
// v0.1.6: Initial: the Standard view's measures and buttons, as the Windows 7 Calculator (D-018)

namespace chrono_calc {

namespace {

// Buttons are 43 x 33; the columns and rows where they start. Standard uses the first five columns; Scientific has
// five more columns of functions to the left of the same ten keys.
// The heights of a digit of the label fonts, measured in Windows 7 (windows7-scientific.md part E): digits Segoe UI
// 20 px, text labels 13 px, = and the backspace arrow 30 px, 1/x 15 px
constexpr double digitInk = 14.0;
constexpr double textInk = 9.0;
constexpr double symbolInk = 21.0;
constexpr double reciprocalInk = 10.5;
constexpr int buttonWidth = 43;
constexpr int buttonHeight = 33;
constexpr std::array rows = {82, 122, 162, 203, 242, 282};
constexpr std::array scientificColumns = {13, 62, 111, 160, 208, 257, 306, 355, 403, 452};
constexpr std::size_t scientificKeysColumn = 5;

constexpr ButtonSpec at(std::string_view commandId, std::string_view label, ButtonFace face, int left, std::size_t row)
{
    return {.commandId = commandId,
            .label = label,
            .inverseLabel = {},
            .rectangle = {.left = left, .top = rows.at(row), .width = buttonWidth, .height = buttonHeight},
            .face = face,
            .labelInkHeight = face == ButtonFace::Digit ? digitInk : textInk};
}

constexpr ButtonSpec inkHeight(ButtonSpec spec, double labelInkHeight)
{
    spec.labelInkHeight = labelInkHeight;
    return spec;
}

constexpr ButtonSpec withInverse(ButtonSpec spec, std::string_view inverseLabel)
{
    spec.inverseLabel = inverseLabel;
    return spec;
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

// The ten keys both views share (memory, clearing, digits, operators), in columns 0 to 4 of the block that starts at
// the column given; the labels are our own text, the decimal point's label is replaced by the regional separator
template <std::size_t first>
constexpr std::array<ButtonSpec, 28> keys(const std::array<int, 10>& columns)
{
    const auto x = [&columns](std::size_t column) { return columns.at(first + column); };
    return {
        at("memory.clear", "MC", ButtonFace::Memory, x(0), 0),
        at("memory.recall", "MR", ButtonFace::Memory, x(1), 0),
        at("memory.store", "MS", ButtonFace::Memory, x(2), 0),
        at("memory.add", "M+", ButtonFace::Memory, x(3), 0),
        at("memory.subtract", "M-", ButtonFace::Memory, x(4), 0),
        inkHeight(at("backspace", "\u2190", ButtonFace::Function, x(0), 1), symbolInk),
        at("clear_entry", "CE", ButtonFace::Function, x(1), 1),
        at("clear", "C", ButtonFace::Function, x(2), 1),
        at("negate", "±", ButtonFace::Function, x(3), 1),
        at("sqrt", "√", ButtonFace::Function, x(4), 1),
        at("digit.7", "7", ButtonFace::Digit, x(0), 2),
        at("digit.8", "8", ButtonFace::Digit, x(1), 2),
        at("digit.9", "9", ButtonFace::Digit, x(2), 2),
        at("divide", "/", ButtonFace::Function, x(3), 2),
        at("percent", "%", ButtonFace::Function, x(4), 2),
        at("digit.4", "4", ButtonFace::Digit, x(0), 3),
        at("digit.5", "5", ButtonFace::Digit, x(1), 3),
        at("digit.6", "6", ButtonFace::Digit, x(2), 3),
        at("multiply", "*", ButtonFace::Function, x(3), 3),
        inkHeight(at("reciprocal", "1/x", ButtonFace::Function, x(4), 3), reciprocalInk),
        at("digit.1", "1", ButtonFace::Digit, x(0), 4),
        at("digit.2", "2", ButtonFace::Digit, x(1), 4),
        at("digit.3", "3", ButtonFace::Digit, x(2), 4),
        at("subtract", "-", ButtonFace::Function, x(3), 4),
        tall(inkHeight(at("equals", "=", ButtonFace::Function, x(4), 4), symbolInk), 74),
        wide(at("digit.0", "0", ButtonFace::Digit, x(0), 5), 92),
        at(decimalCommand, ".", ButtonFace::Digit, x(2), 5),
        at("add", "+", ButtonFace::Function, x(3), 5),
    };
}

constexpr std::array<int, 10> standardColumns = {13, 62, 111, 160, 208, 0, 0, 0, 0, 0};
constexpr std::array standardButtons = keys<0>(standardColumns);

// The five columns of functions of the Scientific view (the cell of the bracket counter has no button)
constexpr std::array<ButtonSpec, 24> scientificFunctions = {
    at("inverse", "Inv", ButtonFace::Toggle, 62, 1),
    withInverse(at("ln", "ln", ButtonFace::Function, 111, 1), "eˣ"),
    at("open_bracket", "(", ButtonFace::Function, 160, 1),
    at("close_bracket", ")", ButtonFace::Function, 208, 1),
    withInverse(at("integer", "Int", ButtonFace::Function, 13, 2), "Frac"),
    withInverse(at("sinh", "sinh", ButtonFace::Function, 62, 2), "sinh⁻¹"),
    withInverse(at("sin", "sin", ButtonFace::Function, 111, 2), "sin⁻¹"),
    at("square", "x²", ButtonFace::Function, 160, 2),
    at("factorial", "n!", ButtonFace::Function, 208, 2),
    withInverse(at("dms", "dms", ButtonFace::Function, 13, 3), "deg"),
    withInverse(at("cosh", "cosh", ButtonFace::Function, 62, 3), "cosh⁻¹"),
    withInverse(at("cos", "cos", ButtonFace::Function, 111, 3), "cos⁻¹"),
    at("power", "xʸ", ButtonFace::Function, 160, 3),
    at("root", "ʸ√x", ButtonFace::Function, 208, 3),
    withInverse(at("pi", "π", ButtonFace::Function, 13, 4), "2*π"),
    withInverse(at("tanh", "tanh", ButtonFace::Function, 62, 4), "tanh⁻¹"),
    withInverse(at("tan", "tan", ButtonFace::Function, 111, 4), "tan⁻¹"),
    at("cube", "x³", ButtonFace::Function, 160, 4),
    at("cube_root", "∛x", ButtonFace::Function, 208, 4),
    at("scientific_notation", "F-E", ButtonFace::Toggle, 13, 5),
    at("exponent", "Exp", ButtonFace::Function, 62, 5),
    at("modulo", "Mod", ButtonFace::Function, 111, 5),
    at("log", "log", ButtonFace::Function, 160, 5),
    at("power_of_ten", "10ˣ", ButtonFace::Function, 208, 5),
};

constexpr std::array scientificKeys = keys<scientificKeysColumn>(scientificColumns);

constexpr std::array<ButtonSpec, scientificFunctions.size() + scientificKeys.size()> joined()
{
    std::array<ButtonSpec, scientificFunctions.size() + scientificKeys.size()> all{};
    std::size_t index = 0;
    for (const ButtonSpec& spec : scientificFunctions) {
        all.at(index++) = spec;
    }
    for (const ButtonSpec& spec : scientificKeys) {
        all.at(index++) = spec;
    }
    return all;
}

constexpr std::array scientificButtons = joined();

constexpr std::array angles = {
    AngleSpec{.commandId = "angle.degrees", .label = "Degrees", .unit = AngleUnit::Degrees, .rectangle = {.left = 22, .top = 88, .width = 80, .height = 21}},
    AngleSpec{.commandId = "angle.radians", .label = "Radians", .unit = AngleUnit::Radians, .rectangle = {.left = 103, .top = 88, .width = 83, .height = 21}},
    AngleSpec{.commandId = "angle.grads", .label = "Grads", .unit = AngleUnit::Grads, .rectangle = {.left = 186, .top = 88, .width = 62, .height = 21}},
};

const ViewLayout standardLayout{
    .clientWidth = 264,
    .clientHeight = 331,
    .displayFrame = {.left = 13, .top = 13, .width = 238, .height = 63},
    .expressionLine = {.left = 17, .top = 16, .width = 230, .height = 18},
    .memoryMark = {.left = 13, .top = 44, .width = 17, .height = 23},
    .mainNumber = {.left = 35, .top = 33, .width = 211, .height = 36},
    .buttons = standardButtons,
    .angleFrame = {},
    .angles = {},
    .bracketCounter = {},
};

const ViewLayout scientificLayout{
    .clientWidth = 508,
    .clientHeight = 331,
    .displayFrame = {.left = 13, .top = 13, .width = 482, .height = 63},
    .expressionLine = {.left = 17, .top = 16, .width = 474, .height = 18},
    .memoryMark = {.left = 13, .top = 44, .width = 17, .height = 23},
    .mainNumber = {.left = 35, .top = 33, .width = 455, .height = 36},
    .buttons = scientificButtons,
    .angleFrame = {.left = 13, .top = 82, .width = 238, .height = 33},
    .angles = angles,
    .bracketCounter = {.left = 13, .top = 122, .width = 43, .height = 33},
};

}  // namespace

const ViewLayout& layoutOf(View view)
{
    return view == View::Scientific ? scientificLayout : standardLayout;
}

}  // namespace chrono_calc
