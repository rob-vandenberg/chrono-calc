#pragma once

#include <QIcon>
#include <QPixmap>

// --- Version ------------------------------------------------------------
// src/ui/programicon.h 0.9.14

// --- Version history ----------------------------------------------------
// v0.9.14: Initial: the program's own icon (packaging/chrono-calc.svg) for the window and the About box

namespace chrono_calc {

// The icon drawn this many pixels wide and high, sharp on a screen with this scale
QPixmap programIconPixmap(int size, qreal devicePixelRatio);

// The icon in the sizes a desktop shows a window's icon in
QIcon programIcon();

}  // namespace chrono_calc
