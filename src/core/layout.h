#pragma once

#include "core/view.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/layout.h 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: The side panel beside every view, and the parts of the Unit conversion (D-025) and Date calculation (D-026)
//          panels and of the Worksheets (D-027)
// v0.5.12: The Standard and Scientific views with History shown (the list of entries in the display)
// v0.4.11: The Statistics view: its buttons and the list inside the display (D-023)
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

// The Statistics view's list inside the display (Windows 7, windows7-statistics.md part A): the list of values, the
// thin line under it, "Count = " and the count, and the two small buttons that move the selection up and down
struct StatisticsSpec
{
    Rectangle list;
    // The height of one row of the list, its dotted line included
    int rowHeight = 0;
    Rectangle divider;
    Rectangle countLabel;
    Rectangle countValue;
    Rectangle upButton;
    Rectangle downButton;
};

// Where bit 0 to 63 of the bit display sits (Windows 7, windows7-programmer.md part A)
Rectangle bitRectangle(int bit);

// The layout of a view, measured on the Windows 7 Calculator (tests/reference/windows7-standard.md,
// windows7-scientific.md, windows7-programmer.md and windows7-statistics.md, part A); an empty expression line has none
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
    // Statistics only (an empty list elsewhere): the list of values in the display
    StatisticsSpec statistics;
    // Standard and Scientific with History shown (an empty list elsewhere): the list of entries, the same kind of list
    // as the Statistics view's, without a count
    StatisticsSpec history;
};

// The layout of a view; Standard and Scientific are taller with History shown (windows7-history.md part A)
const ViewLayout& layoutOf(View view, bool historyShown = false);

// The side panel beside the calculator (View > Unit conversion, Date calculation, Worksheets), measured on the Windows 7
// Calculator (windows7-unitconversion.md part A): 454 px wide, 13 px right of the view and from the window's top; the
// window grows by the panel and a 13 px margin, its height stays the view's
struct SidePanelLayout
{
    Rectangle frame;
    int clientWidth = 0;
};

SidePanelLayout sidePanelLayoutOf(View view, bool historyShown = false);

// The parts of the Unit conversion panel, from the panel's top left corner (windows7-unitconversion.md part A line 2)
struct UnitConversionLayout
{
    Rectangle typeLabel;
    Rectangle typeList;
    Rectangle fromLabel;
    Rectangle fromBox;
    Rectangle fromList;
    Rectangle toLabel;
    Rectangle toBox;
    Rectangle toList;
};

const UnitConversionLayout& unitConversionLayout();

// The parts of the Date calculation panel, from the panel's top left corner (windows7-datecalculation.md part A line 2):
// the mode label and list are always there, the rest belongs to one mode
struct DateCalculationLayout
{
    Rectangle modeLabel;
    Rectangle modeList;
    // Calculate the difference between two dates
    Rectangle differenceFromLabel;
    Rectangle differenceFrom;
    Rectangle differenceToLabel;
    Rectangle differenceTo;
    Rectangle differenceLabel;
    Rectangle difference;
    Rectangle totalDaysLabel;
    Rectangle totalDays;
    // Add or subtract days to a specified date; the number boxes in the order Year(s), Month(s), Day(s)
    Rectangle shiftFromLabel;
    Rectangle shiftFrom;
    Rectangle add;
    Rectangle subtract;
    std::array<Rectangle, 3> fieldLabels;
    std::array<Rectangle, 3> fields;
    std::array<Rectangle, 3> fieldArrows;
    Rectangle shiftedLabel;
    Rectangle shifted;
    // The same place in both modes
    Rectangle calculate;
};

const DateCalculationLayout& dateCalculationLayout();

// The most boxes a Worksheet has (Vehicle lease: five values and the one calculated)
inline constexpr std::size_t mostWorksheetBoxes = 5;

// The parts of a Worksheet, from the panel's top left corner (windows7-worksheets.md part A line 3): the same places in
// all four; a worksheet with fewer boxes uses the first ones
struct WorksheetLayout
{
    Rectangle choiceLabel;
    Rectangle choiceList;
    std::array<Rectangle, mostWorksheetBoxes> valueLabels;
    std::array<Rectangle, mostWorksheetBoxes> boxes;
    Rectangle calculate;
    Rectangle result;
};

const WorksheetLayout& worksheetLayout();

// The command id of the decimal-point button; the window shows the regional decimal separator as its label
inline constexpr std::string_view decimalCommand = "decimal";

}  // namespace chrono_calc
