#pragma once

#include <cstdint>

// --- Version ------------------------------------------------------------
// src/core/colours.h 0.9.14

// --- Version history ----------------------------------------------------
// v0.9.14: The About box: its white face, the blue name in the top band, the thin grey line, the text and the links
// v0.8.13: The side panel and the Unit conversion panel's labels, lists and boxes; the Date calculation panel's date
//          boxes and Calculate button; the Worksheets' hint in an empty box and their message balloon
// v0.5.12: The edit box of History and the Statistics list
// v0.4.11: The Statistics view's list, count and selection buttons
// v0.3.8: The flat face of A to F, the bit display; the choice colours serve angle units, bases and word sizes
// v0.2.7: The faces of function, memory, toggle and greyed buttons; the angle units and the bracket counter
// v0.1.6: Initial: the colours of the Standard view, close to the Windows 7 Calculator (D-005, D-018)

namespace chrono_calc {

// A colour as red, green and blue (0 to 255), and how much it covers what lies below (255: fully)
struct Colour
{
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
    std::uint8_t opacity = 255;
};

// Every colour of the program lives here and nowhere else (AGENTS.md 5.6); nothing comes from the platform theme.
// The values are those measured in the Windows 7 Calculator with the Aero look (tests/reference/windows7-standard.md
// part G, windows7-scientific.md part E), except the menu highlight, the hover and pressed glow, the greyed label and
// the angle circle edge, which are our own close to it (D-018, D-020).
namespace colours {

// The window behind the display and the buttons: a soft blue, lighter at the top
inline constexpr Colour windowTop{.red = 0xEC, .green = 0xF4, .blue = 0xFC};
inline constexpr Colour windowUpper{.red = 0xDC, .green = 0xE7, .blue = 0xF3};
inline constexpr Colour windowBottom{.red = 0xD9, .green = 0xE4, .blue = 0xF1};
// The menu bar and the menus
inline constexpr Colour menuBackground{.red = 0xFF, .green = 0xFF, .blue = 0xFF};
inline constexpr Colour menuText{.red = 0x00, .green = 0x00, .blue = 0x00};
inline constexpr Colour menuHighlight{.red = 0xC4, .green = 0xD9, .blue = 0xF2};
inline constexpr Colour menuDisabledText{.red = 0x8D, .green = 0x8D, .blue = 0x8D};
inline constexpr Colour menuBorder{.red = 0x97, .green = 0x97, .blue = 0x97};
// The small hover texts
inline constexpr Colour toolTipBackground{.red = 0xFF, .green = 0xFF, .blue = 0xFF};
inline constexpr Colour toolTipText{.red = 0x57, .green = 0x57, .blue = 0x57};

// The display: a frame with a white line inside, its face pale blue at the top and white at the bottom
inline constexpr Colour displayBorder{.red = 0x8E, .green = 0x9C, .blue = 0xAD};
inline constexpr Colour displayInnerLine{.red = 0xFF, .green = 0xFF, .blue = 0xFF};
inline constexpr Colour displayTop{.red = 0xE5, .green = 0xEE, .blue = 0xFB};
inline constexpr Colour displayMiddle{.red = 0xF0, .green = 0xF5, .blue = 0xFC};
inline constexpr Colour displayBottom{.red = 0xFF, .green = 0xFF, .blue = 0xFF};
inline constexpr Colour displayNumber{.red = 0x33, .green = 0x33, .blue = 0x33};
inline constexpr Colour displayExpression{.red = 0x33, .green = 0x33, .blue = 0x33};
inline constexpr Colour displayMemoryMark{.red = 0x33, .green = 0x33, .blue = 0x33};

// A digit button at rest: a light face, darker in the middle, with a blue-grey edge and a thin light line just inside it
inline constexpr Colour buttonBorder{.red = 0x87, .green = 0x97, .blue = 0xAA};
inline constexpr Colour buttonInnerLine{.red = 0xFF, .green = 0xFF, .blue = 0xFF, .opacity = 0xA0};
inline constexpr Colour buttonTop{.red = 0xF7, .green = 0xFB, .blue = 0xFF};
inline constexpr Colour buttonMiddle{.red = 0xE7, .green = 0xEE, .blue = 0xF6};
inline constexpr Colour buttonBottom{.red = 0xED, .green = 0xF5, .blue = 0xFB};
inline constexpr Colour buttonText{.red = 0x1E, .green = 0x39, .blue = 0x5B};
// A function or operator button (also C, CE, = and the Scientific functions): a darker, bluer face
inline constexpr Colour functionTop{.red = 0xF0, .green = 0xF6, .blue = 0xFB};
inline constexpr Colour functionMiddle{.red = 0xD4, .green = 0xE0, .blue = 0xED};
inline constexpr Colour functionBottom{.red = 0xD7, .green = 0xE2, .blue = 0xF0};
// A memory button: the function colours, flat, with a light first line
inline constexpr Colour memoryTop{.red = 0xF0, .green = 0xF4, .blue = 0xF9};
inline constexpr Colour memoryFace{.red = 0xD4, .green = 0xE0, .blue = 0xED};
inline constexpr Colour memoryBottom{.red = 0xD7, .green = 0xE2, .blue = 0xF0};
// A toggle that is on (Inv, F-E): a warm face
inline constexpr Colour toggleOnBorder{.red = 0xA0, .green = 0x99, .blue = 0x74};
inline constexpr Colour toggleOnTop{.red = 0xF0, .green = 0xE5, .blue = 0xCE};
inline constexpr Colour toggleOnMiddle{.red = 0xE2, .green = 0xD0, .blue = 0xAE};
inline constexpr Colour toggleOnBottom{.red = 0xE2, .green = 0xE9, .blue = 0xDF};
// A greyed button (% in the Scientific view): a light edge on the window's own colour
inline constexpr Colour greyedBorder{.red = 0xB4, .green = 0xC2, .blue = 0xD1};
inline constexpr Colour greyedFace{.red = 0xD9, .green = 0xE4, .blue = 0xF1};
inline constexpr Colour greyedText{.red = 0x8D, .green = 0x9B, .blue = 0xAB};
// The round choices (angle units, number bases, word sizes) in their flat frames: white circles, a dark dot on the
// chosen one
inline constexpr Colour frameBorder{.red = 0x86, .green = 0x96, .blue = 0xAA};
inline constexpr Colour frameFace{.red = 0xD9, .green = 0xE4, .blue = 0xF1};
inline constexpr Colour choiceText{.red = 0x1E, .green = 0x39, .blue = 0x5B};
inline constexpr Colour choiceCircle{.red = 0xFF, .green = 0xFF, .blue = 0xFF};
inline constexpr Colour choiceCircleBorder{.red = 0x8E, .green = 0x9C, .blue = 0xAD};
inline constexpr Colour choiceDot{.red = 0x33, .green = 0x33, .blue = 0x33};
// The hexadecimal digits A to F: a flat face
inline constexpr Colour flatFace{.red = 0xD9, .green = 0xE4, .blue = 0xF1};
// The bit display: the bits and, lighter, the bit positions under them
inline constexpr Colour bitText{.red = 0x1E, .green = 0x39, .blue = 0x5B};
inline constexpr Colour bitPositionText{.red = 0x66, .green = 0x66, .blue = 0x66};
// The bracket counter "(=1": a flat cell
inline constexpr Colour counterBorder{.red = 0x8E, .green = 0x9C, .blue = 0xAD};
inline constexpr Colour counterFace{.red = 0xD9, .green = 0xE4, .blue = 0xF1};
inline constexpr Colour counterText{.red = 0x1E, .green = 0x39, .blue = 0x5B};
// The Statistics view's list inside the display (windows7-statistics.md part A): grey values on the display's own
// face, a dotted line under each row, the selected row light blue; a thin line above the count; the small flat
// buttons that move the selection, with a dark triangle
inline constexpr Colour listText{.red = 0x59, .green = 0x6B, .blue = 0x72};
inline constexpr Colour listSelected{.red = 0x9E, .green = 0xCB, .blue = 0xF5};
inline constexpr Colour listRowLine{.red = 0x74, .green = 0xAA, .blue = 0xBD};
inline constexpr Colour listDivider{.red = 0xD1, .green = 0xD7, .blue = 0xDE};
inline constexpr Colour countText{.red = 0x1E, .green = 0x39, .blue = 0x5B};
inline constexpr Colour arrowFace{.red = 0xE2, .green = 0xED, .blue = 0xFA};
inline constexpr Colour arrowGlyph{.red = 0x1E, .green = 0x39, .blue = 0x5B};
// The edit box over a row of the history or the Statistics list (Edit, F2): Windows' plain edit box, white with black
// text (not measured: Windows 7 draws it with the system's own colours), the selected text in the list's blue
inline constexpr Colour editBoxFace{.red = 0xFF, .green = 0xFF, .blue = 0xFF};
inline constexpr Colour editBoxText{.red = 0x00, .green = 0x00, .blue = 0x00};
// The side panel beside the calculator (windows7-unitconversion.md part A line 3): flat white inside a thin blue-grey
// frame, dark blue labels; its lists with a light grey face, a grey edge and a dark arrow; the boxes white (From) or
// light grey (To, read-only) with a grey edge, blue while the box has the keyboard focus, black text
inline constexpr Colour panelFace{.red = 0xFF, .green = 0xFF, .blue = 0xFF};
inline constexpr Colour panelFrame{.red = 0x88, .green = 0x9A, .blue = 0xAD};
inline constexpr Colour panelLabel{.red = 0x1E, .green = 0x39, .blue = 0x5B};
inline constexpr Colour panelListFace{.red = 0xE1, .green = 0xE1, .blue = 0xE1};
inline constexpr Colour panelListEdge{.red = 0xAD, .green = 0xAD, .blue = 0xAD};
inline constexpr Colour panelListArrow{.red = 0x56, .green = 0x56, .blue = 0x56};
inline constexpr Colour panelBoxFace{.red = 0xFF, .green = 0xFF, .blue = 0xFF};
inline constexpr Colour panelReadOnlyFace{.red = 0xF0, .green = 0xF0, .blue = 0xF0};
inline constexpr Colour panelBoxEdge{.red = 0x7A, .green = 0x7A, .blue = 0x7A};
inline constexpr Colour panelBoxFocusEdge{.red = 0x00, .green = 0x78, .blue = 0xD7};
inline constexpr Colour panelText{.red = 0x00, .green = 0x00, .blue = 0x00};
// The Date calculation panel (windows7-datecalculation.md part A line 3): the date boxes white with the boxes' grey edge
// and a blue-grey arrow; the Calculate button light grey with the blue edge of the button Enter presses
inline constexpr Colour panelDateArrow{.red = 0x4D, .green = 0x61, .blue = 0x85};
inline constexpr Colour panelButtonFace{.red = 0xE1, .green = 0xE1, .blue = 0xE1};
inline constexpr Colour panelButtonEdge{.red = 0x00, .green = 0x78, .blue = 0xD7};
// The Worksheets (windows7-worksheets.md part A line 3): the grey hint "Enter value" in an empty box, and the balloon of
// a message, white with a grey edge and black text (our own, not measured)
inline constexpr Colour panelHint{.red = 0x6D, .green = 0x6D, .blue = 0x6D};
inline constexpr Colour panelBalloonFace{.red = 0xFF, .green = 0xFF, .blue = 0xFF};
inline constexpr Colour panelBalloonEdge{.red = 0x76, .green = 0x76, .blue = 0x76};
// The About box (Help > About Chrono Calc, D-028), laid out like Windows 10's About box: white, the name in the top band
// in the icon's blue, a thin grey line below the band, black text, the links in the icon's blue (our own, not measured)
inline constexpr Colour aboutFace{.red = 0xFF, .green = 0xFF, .blue = 0xFF};
inline constexpr Colour aboutTitle{.red = 0x2B, .green = 0x7B, .blue = 0xD6};
inline constexpr Colour aboutLine{.red = 0xD0, .green = 0xD0, .blue = 0xD0};
inline constexpr Colour aboutText{.red = 0x00, .green = 0x00, .blue = 0x00};
inline constexpr Colour aboutLink{.red = 0x2B, .green = 0x7B, .blue = 0xD6};
// A button under the mouse: a warm golden glow
inline constexpr Colour buttonHoverBorder{.red = 0xC2, .green = 0x9B, .blue = 0x45};
inline constexpr Colour buttonHoverTop{.red = 0xFF, .green = 0xF4, .blue = 0xD4};
inline constexpr Colour buttonHoverMiddle{.red = 0xFF, .green = 0xE2, .blue = 0x94};
inline constexpr Colour buttonHoverBottom{.red = 0xFF, .green = 0xD2, .blue = 0x6A};
// A button held down: a deeper orange
inline constexpr Colour buttonPressedBorder{.red = 0x9A, .green = 0x6E, .blue = 0x2E};
inline constexpr Colour buttonPressedTop{.red = 0xF7, .green = 0xC9, .blue = 0x7C};
inline constexpr Colour buttonPressedMiddle{.red = 0xF5, .green = 0xB4, .blue = 0x4E};
inline constexpr Colour buttonPressedBottom{.red = 0xF8, .green = 0xCB, .blue = 0x6E};

}  // namespace colours

}  // namespace chrono_calc
