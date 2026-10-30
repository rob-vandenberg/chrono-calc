#pragma once

#include <QFont>

// --- Version ------------------------------------------------------------
// src/ui/fonts.h 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: Initial: fonts sized by the height of their digits, as measured in Windows 7

namespace chrono_calc {

// This font in the pixel size at which its digits are this many pixels high. Windows 7's fonts are not on Linux; sizing
// the computer's own font by the height of its figures keeps the figures as large as in Windows 7, whatever font it is.
QFont fontWithInkHeight(QFont font, double inkHeight);

// The computer's fixed-width font (the display's figures; Windows 7 uses a fixed-width font there)
QFont fixedWidthFont();

}  // namespace chrono_calc
