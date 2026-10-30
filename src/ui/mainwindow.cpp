#include "ui/mainwindow.h"

// --- Version ------------------------------------------------------------
// src/ui/mainwindow.cpp 0.0.2

// --- Version history ----------------------------------------------------
// v0.0.2: Initial: an empty window titled Chrono Calc

namespace chrono_calc {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Chrono Calc"));
}

}  // namespace chrono_calc
