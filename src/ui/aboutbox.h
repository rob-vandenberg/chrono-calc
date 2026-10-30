#pragma once

#include <QDialog>

// --- Version ------------------------------------------------------------
// src/ui/aboutbox.h 0.9.14

// --- Version history ----------------------------------------------------
// v0.9.14: Initial: Help > About Chrono Calc (D-028)

class QWidget;

namespace chrono_calc {

// Help > About Chrono Calc (F1, D-028), laid out like Windows 10's About box: a top band with the icon and the name in
// large blue letters, a thin grey line, then the small icon beside the name, the version, the copyright, the licence,
// the warranty disclaimer and the project's page; OK at the bottom right (OK, Enter and Escape close it). A fixed
// size, in our own colours only.
class AboutBox : public QDialog
{
public:
    explicit AboutBox(QWidget* parent);
};

}  // namespace chrono_calc
