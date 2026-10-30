#pragma once

#include <QColor>

#include "core/colours.h"


// --- Version ------------------------------------------------------------
// src/ui/look.h 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: Initial: Qt colours from colours.h, and the program's own look on every desktop (D-018)

namespace chrono_calc {

// The Qt colour of one of our colours
inline QColor qtColour(Colour colour)
{
    return {colour.red, colour.green, colour.blue, colour.opacity};
}

// Gives the program its own look: Qt's plain Fusion drawing with only our colours, so nothing comes from the
// desktop's theme (AGENTS.md 5.6) and the calculator looks the same on KDE and GNOME
void applyLook();

}  // namespace chrono_calc
