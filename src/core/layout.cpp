#include "core/layout.h"

#include "core/view.h"

#include <array>
#include <cstddef>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/layout.cpp 0.5.12

// --- Version history ----------------------------------------------------
// v0.5.12: The Standard and Scientific views with History shown: the list in a taller display, everything else lower
// v0.4.11: The Statistics view: its buttons and the list inside the display (D-023)
// v0.3.8: The Programmer view: its buttons, the bases and word sizes, the bit display; choices of any kind
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
// The statistic buttons of the Statistics view: Segoe UI Symbol 17 px
constexpr double statisticInk = 12.0;
constexpr int buttonWidth = 43;
constexpr int buttonHeight = 33;
constexpr std::array<int, 6> rows = {82, 122, 162, 203, 242, 282};
constexpr std::array<int, 10> scientificColumns = {13, 62, 111, 160, 208, 257, 306, 355, 403, 452};
constexpr std::size_t scientificKeysColumn = 5;
// The Programmer view sits lower: the bit display takes the top rows
constexpr std::array<int, 6> programmerRows = {165, 205, 245, 284, 325, 364};
constexpr std::array<int, 10> programmerColumns = {111, 160, 208, 257, 306, 355, 403, 452, 0, 0};
constexpr std::size_t programmerKeysColumn = 3;

constexpr ButtonSpec placed(std::string_view commandId, std::string_view label, ButtonFace face, int left, int top)
{
    return {.commandId = commandId,
            .label = label,
            .inverseLabel = {},
            .rectangle = {.left = left, .top = top, .width = buttonWidth, .height = buttonHeight},
            .face = face,
            .labelInkHeight = face == ButtonFace::Digit || face == ButtonFace::Flat ? digitInk : textInk};
}

// A button of the Standard or Scientific view, in one of their rows
constexpr ButtonSpec at(std::string_view commandId, std::string_view label, ButtonFace face, int left, std::size_t row)
{
    return placed(commandId, label, face, left, rows.at(row));
}

// A button of the Programmer view, in one of its rows
constexpr ButtonSpec low(std::string_view commandId, std::string_view label, ButtonFace face, int left, std::size_t row)
{
    return placed(commandId, label, face, left, programmerRows.at(row));
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
constexpr std::array<ButtonSpec, 28> keys(const std::array<int, 10>& columns, std::size_t first, const std::array<int, 6>& tops)
{
    const auto x = [&columns, first](std::size_t column) { return columns.at(first + column); };
    const auto at = [&tops](std::string_view commandId, std::string_view label, ButtonFace face, int left, std::size_t row) {
        return placed(commandId, label, face, left, tops.at(row));
    };
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
constexpr std::array standardButtons = keys(standardColumns, 0, rows);

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

constexpr std::array scientificKeys = keys(scientificColumns, scientificKeysColumn, rows);

template <std::size_t functionCount>
constexpr std::array<ButtonSpec, functionCount + 28> joined(const std::array<ButtonSpec, functionCount>& functions,
                                                            const std::array<ButtonSpec, 28>& shared)
{
    std::array<ButtonSpec, functionCount + 28> all{};
    std::size_t index = 0;
    for (const ButtonSpec& spec : functions) {
        all.at(index++) = spec;
    }
    for (const ButtonSpec& spec : shared) {
        all.at(index++) = spec;
    }
    return all;
}

constexpr std::array scientificButtons = joined(scientificFunctions, scientificKeys);

constexpr std::array<ChoiceSpec, 3> angles = {{
    {.commandId = "angle.degrees", .label = "Degrees", .rectangle = {.left = 22, .top = 88, .width = 80, .height = 21}},
    {.commandId = "angle.radians", .label = "Radians", .rectangle = {.left = 103, .top = 88, .width = 83, .height = 21}},
    {.commandId = "angle.grads", .label = "Grads", .rectangle = {.left = 186, .top = 88, .width = 62, .height = 21}},
}};
constexpr std::array<Rectangle, 1> scientificFrames = {{{.left = 13, .top = 82, .width = 238, .height = 33}}};

// The Programmer view: three columns of its own to the left of the shared keys (the counter cell has no button)
constexpr std::array<ButtonSpec, 17> programmerFunctions = {
    low("modulo", "Mod", ButtonFace::Function, 160, 0),
    low("digit.a", "A", ButtonFace::Flat, 208, 0),
    low("open_bracket", "(", ButtonFace::Function, 111, 1),
    low("close_bracket", ")", ButtonFace::Function, 160, 1),
    low("digit.b", "B", ButtonFace::Flat, 208, 1),
    low("rotate_left", "RoL", ButtonFace::Function, 111, 2),
    low("rotate_right", "RoR", ButtonFace::Function, 160, 2),
    low("digit.c", "C", ButtonFace::Flat, 208, 2),
    low("or", "Or", ButtonFace::Function, 111, 3),
    low("xor", "Xor", ButtonFace::Function, 160, 3),
    low("digit.d", "D", ButtonFace::Flat, 208, 3),
    low("shift_left", "Lsh", ButtonFace::Function, 111, 4),
    low("shift_right", "Rsh", ButtonFace::Function, 160, 4),
    low("digit.e", "E", ButtonFace::Flat, 208, 4),
    low("not", "Not", ButtonFace::Function, 111, 5),
    low("and", "And", ButtonFace::Function, 160, 5),
    low("digit.f", "F", ButtonFace::Flat, 208, 5),
};

constexpr std::array programmerKeys = keys(programmerColumns, programmerKeysColumn, programmerRows);
constexpr std::array programmerButtons = joined(programmerFunctions, programmerKeys);

constexpr std::array<ChoiceSpec, 8> programmerChoices = {{
    {.commandId = "base.hexadecimal", .label = "Hex", .rectangle = {.left = 22, .top = 175, .width = 69, .height = 19}},
    {.commandId = "base.decimal", .label = "Dec", .rectangle = {.left = 22, .top = 200, .width = 73, .height = 19}},
    {.commandId = "base.octal", .label = "Oct", .rectangle = {.left = 22, .top = 225, .width = 75, .height = 19}},
    {.commandId = "base.binary", .label = "Bin", .rectangle = {.left = 22, .top = 250, .width = 75, .height = 19}},
    {.commandId = "word.qword", .label = "Qword", .rectangle = {.left = 22, .top = 293, .width = 79, .height = 19}},
    {.commandId = "word.dword", .label = "Dword", .rectangle = {.left = 22, .top = 318, .width = 79, .height = 19}},
    {.commandId = "word.word", .label = "Word", .rectangle = {.left = 22, .top = 343, .width = 73, .height = 19}},
    {.commandId = "word.byte", .label = "Byte", .rectangle = {.left = 22, .top = 368, .width = 72, .height = 19}},
}};
constexpr std::array<Rectangle, 2> programmerFrames = {{
    {.left = 13, .top = 165, .width = 92, .height = 115},
    {.left = 13, .top = 284, .width = 92, .height = 115},
}};

constexpr int bitLabelWidth = 18;
constexpr int bitLabelHeight = 17;
constexpr std::array<BitLabelSpec, 6> bitLabels = {{
    {.text = "63", .rectangle = {.left = 22, .top = 103, .width = bitLabelWidth, .height = bitLabelHeight}, .alignRight = false},
    {.text = "47", .rectangle = {.left = 262, .top = 103, .width = bitLabelWidth, .height = bitLabelHeight}, .alignRight = false},
    {.text = "32", .rectangle = {.left = 461, .top = 103, .width = bitLabelWidth, .height = bitLabelHeight}, .alignRight = true},
    {.text = "31", .rectangle = {.left = 22, .top = 136, .width = bitLabelWidth, .height = bitLabelHeight}, .alignRight = false},
    {.text = "15", .rectangle = {.left = 262, .top = 136, .width = bitLabelWidth, .height = bitLabelHeight}, .alignRight = false},
    {.text = "0", .rectangle = {.left = 461, .top = 136, .width = bitLabelWidth, .height = bitLabelHeight}, .alignRight = true},
}};

// The bits: two rows (bits 63 to 32, then 31 to 0) of eight groups of four
constexpr std::array<int, 2> bitRowTops = {88, 121};
constexpr std::array<int, 8> bitGroupLefts = {22, 82, 142, 202, 262, 322, 382, 442};
constexpr std::array<int, 4> bitOffsets = {0, 10, 19, 29};
constexpr int bitWidth = 9;
constexpr int bitHeight = 15;
constexpr int bitsPerRow = 32;
constexpr int bitsPerGroup = 4;

// The Statistics view (windows7-statistics.md part A): as narrow as Standard; the display holds the list
constexpr std::array<int, 6> statisticsRows = {207, 247, 287, 328, 367, 407};

constexpr ButtonSpec stat(std::string_view commandId, std::string_view label, ButtonFace face, int left, std::size_t row)
{
    return placed(commandId, label, face, left, statisticsRows.at(row));
}

constexpr std::array<ButtonSpec, 29> statisticsButtons = {
    stat("memory.clear", "MC", ButtonFace::Memory, 13, 0),
    stat("memory.recall", "MR", ButtonFace::Memory, 62, 0),
    stat("memory.store", "MS", ButtonFace::Memory, 111, 0),
    stat("memory.add", "M+", ButtonFace::Memory, 160, 0),
    stat("memory.subtract", "M-", ButtonFace::Memory, 208, 0),
    inkHeight(stat("backspace", "\u2190", ButtonFace::Function, 13, 1), symbolInk),
    stat("statistics.clear_all", "CAD", ButtonFace::Function, 62, 1),
    stat("clear", "C", ButtonFace::Function, 111, 1),
    stat("scientific_notation", "F-E", ButtonFace::Toggle, 160, 1),
    stat("exponent", "Exp", ButtonFace::Function, 208, 1),
    stat("digit.7", "7", ButtonFace::Digit, 13, 2),
    stat("digit.8", "8", ButtonFace::Digit, 62, 2),
    stat("digit.9", "9", ButtonFace::Digit, 111, 2),
    inkHeight(stat("statistics.mean", "x̄", ButtonFace::Function, 160, 2), statisticInk),
    inkHeight(stat("statistics.mean_of_squares", "x̄²", ButtonFace::Function, 208, 2), statisticInk),
    stat("digit.4", "4", ButtonFace::Digit, 13, 3),
    stat("digit.5", "5", ButtonFace::Digit, 62, 3),
    stat("digit.6", "6", ButtonFace::Digit, 111, 3),
    inkHeight(stat("statistics.sum", "Σx", ButtonFace::Function, 160, 3), statisticInk),
    inkHeight(stat("statistics.sum_of_squares", "Σx²", ButtonFace::Function, 208, 3), statisticInk),
    stat("digit.1", "1", ButtonFace::Digit, 13, 4),
    stat("digit.2", "2", ButtonFace::Digit, 62, 4),
    stat("digit.3", "3", ButtonFace::Digit, 111, 4),
    inkHeight(stat("statistics.deviation", "σₙ", ButtonFace::Function, 160, 4), statisticInk),
    inkHeight(stat("statistics.sample_deviation", "σₙ₋₁", ButtonFace::Function, 208, 4), statisticInk),
    wide(stat("digit.0", "0", ButtonFace::Digit, 13, 5), 92),
    // The decimal point has the digit face but a small label (Segoe UI 13 px)
    inkHeight(stat(decimalCommand, ".", ButtonFace::Digit, 111, 5), textInk),
    stat("negate", "±", ButtonFace::Function, 160, 5),
    stat("statistics.add", "Add", ButtonFace::Function, 208, 5),
};

const ViewLayout standardLayout{
    .clientWidth = 264,
    .clientHeight = 331,
    .displayFrame = {.left = 13, .top = 13, .width = 238, .height = 63},
    .expressionLine = {.left = 17, .top = 16, .width = 230, .height = 18},
    .memoryMark = {.left = 13, .top = 44, .width = 17, .height = 23},
    .mainNumber = {.left = 35, .top = 33, .width = 211, .height = 36},
    .buttons = standardButtons,
    .frames = {},
    .choices = {},
    .bracketCounter = {},
    .bitDisplay = {},
    .statistics = {},
    .history = {},
};

const ViewLayout scientificLayout{
    .clientWidth = 508,
    .clientHeight = 331,
    .displayFrame = {.left = 13, .top = 13, .width = 482, .height = 63},
    .expressionLine = {.left = 17, .top = 16, .width = 474, .height = 18},
    .memoryMark = {.left = 13, .top = 44, .width = 17, .height = 23},
    .mainNumber = {.left = 35, .top = 33, .width = 455, .height = 36},
    .buttons = scientificButtons,
    .frames = scientificFrames,
    .choices = angles,
    .bracketCounter = {.left = 13, .top = 122, .width = 43, .height = 33},
    .bitDisplay = {},
    .statistics = {},
    .history = {},
};

const ViewLayout programmerLayout{
    .clientWidth = 508,
    .clientHeight = 413,
    .displayFrame = {.left = 13, .top = 13, .width = 482, .height = 63},
    .expressionLine = {.left = 17, .top = 16, .width = 474, .height = 18},
    .memoryMark = {.left = 13, .top = 44, .width = 17, .height = 23},
    .mainNumber = {.left = 35, .top = 33, .width = 455, .height = 36},
    .buttons = programmerButtons,
    .frames = programmerFrames,
    .choices = programmerChoices,
    .bracketCounter = {.left = 111, .top = 165, .width = 43, .height = 33},
    .bitDisplay = {.frame = {.left = 13, .top = 82, .width = 482, .height = 77}, .labels = bitLabels},
    .statistics = {},
    .history = {},
};

// The display is 188 px high: the selection buttons, the list (five rows of 18 px), a thin line, the count, and the
// number with the M at the bottom; no expression line
const ViewLayout statisticsLayout{
    .clientWidth = 264,
    .clientHeight = 456,
    .displayFrame = {.left = 13, .top = 13, .width = 238, .height = 188},
    .expressionLine = {},
    .memoryMark = {.left = 19, .top = 171, .width = 17, .height = 23},
    .mainNumber = {.left = 35, .top = 159, .width = 208, .height = 36},
    .buttons = statisticsButtons,
    .frames = {},
    .choices = {},
    .bracketCounter = {},
    .bitDisplay = {},
    .statistics = {.list = {.left = 21, .top = 47, .width = 224, .height = 91},
                   .rowHeight = 18,
                   .divider = {.left = 17, .top = 140, .width = 230, .height = 1},
                   .countLabel = {.left = 19, .top = 143, .width = 47, .height = 19},
                   .countValue = {.left = 66, .top = 143, .width = 66, .height = 19},
                   .upButton = {.left = 194, .top = 21, .width = 22, .height = 21},
                   .downButton = {.left = 221, .top = 21, .width = 22, .height = 21}},
    .history = {},
};

// With History shown (windows7-history.md part A), the Standard and Scientific displays are 125 px taller: the list of
// entries with its two selection buttons at the top, a thin line, then the expression line, the M and the number;
// everything below the display is 125 px lower
constexpr int historyHeight = 125;

constexpr Rectangle lowered(Rectangle rectangle)
{
    rectangle.top += historyHeight;
    return rectangle;
}

template <std::size_t count>
constexpr std::array<ButtonSpec, count> loweredButtons(const std::array<ButtonSpec, count>& buttons)
{
    std::array<ButtonSpec, count> result = buttons;
    for (ButtonSpec& button : result) {
        button.rectangle = lowered(button.rectangle);
    }
    return result;
}

template <std::size_t count>
constexpr std::array<ChoiceSpec, count> loweredChoices(const std::array<ChoiceSpec, count>& choices)
{
    std::array<ChoiceSpec, count> result = choices;
    for (ChoiceSpec& choice : result) {
        choice.rectangle = lowered(choice.rectangle);
    }
    return result;
}

constexpr std::array standardHistoryButtons = loweredButtons(standardButtons);
constexpr std::array scientificHistoryButtons = loweredButtons(scientificButtons);
constexpr std::array scientificHistoryChoices = loweredChoices(angles);
constexpr std::array<Rectangle, 1> scientificHistoryFrames = {lowered(scientificFrames.front())};

// The history list of a display this wide (238 in Standard, 482 in Scientific)
constexpr StatisticsSpec historyList(int displayWidth)
{
    const int listWidth = displayWidth - 14;
    return {.list = {.left = 20, .top = 46, .width = listWidth, .height = 91},
            .rowHeight = 18,
            .divider = {.left = 17, .top = 138, .width = displayWidth - 8, .height = 1},
            .countLabel = {},
            .countValue = {},
            .upButton = {.left = listWidth - 27, .top = 20, .width = 22, .height = 21},
            .downButton = {.left = listWidth, .top = 20, .width = 22, .height = 21}};
}

const ViewLayout standardHistoryLayout{
    .clientWidth = 264,
    .clientHeight = 456,
    .displayFrame = {.left = 13, .top = 13, .width = 238, .height = 188},
    .expressionLine = {.left = 17, .top = 141, .width = 230, .height = 18},
    .memoryMark = {.left = 13, .top = 169, .width = 17, .height = 23},
    .mainNumber = {.left = 35, .top = 158, .width = 211, .height = 36},
    .buttons = standardHistoryButtons,
    .frames = {},
    .choices = {},
    .bracketCounter = {},
    .bitDisplay = {},
    .statistics = {},
    .history = historyList(238),
};

const ViewLayout scientificHistoryLayout{
    .clientWidth = 508,
    .clientHeight = 456,
    .displayFrame = {.left = 13, .top = 13, .width = 482, .height = 188},
    .expressionLine = {.left = 17, .top = 141, .width = 474, .height = 18},
    .memoryMark = {.left = 13, .top = 169, .width = 17, .height = 23},
    .mainNumber = {.left = 35, .top = 158, .width = 455, .height = 36},
    .buttons = scientificHistoryButtons,
    .frames = scientificHistoryFrames,
    .choices = scientificHistoryChoices,
    .bracketCounter = lowered({.left = 13, .top = 122, .width = 43, .height = 33}),
    .bitDisplay = {},
    .statistics = {},
    .history = historyList(482),
};

}  // namespace

const ViewLayout& layoutOf(View view, bool historyShown)
{
    if (historyShown && view == View::Standard) {
        return standardHistoryLayout;
    }
    if (historyShown && view == View::Scientific) {
        return scientificHistoryLayout;
    }
    switch (view) {
    case View::Scientific: return scientificLayout;
    case View::Programmer: return programmerLayout;
    case View::Statistics: return statisticsLayout;
    case View::Standard: break;
    }
    return standardLayout;
}

Rectangle bitRectangle(int bit)
{
    // Bit 63 is the first (top left); a row holds 32 bits in eight groups of four
    const int fromLeft = 63 - bit;
    const int row = fromLeft / bitsPerRow;
    const int inRow = fromLeft % bitsPerRow;
    return {.left = bitGroupLefts.at(static_cast<std::size_t>(inRow / bitsPerGroup)) + bitOffsets.at(static_cast<std::size_t>(inRow % bitsPerGroup)),
            .top = bitRowTops.at(static_cast<std::size_t>(row)),
            .width = bitWidth,
            .height = bitHeight};
}

}  // namespace chrono_calc
