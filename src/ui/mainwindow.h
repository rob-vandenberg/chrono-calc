#pragma once

#include <QMainWindow>

// --- Version ------------------------------------------------------------
// src/ui/mainwindow.h 0.0.2

// --- Version history ----------------------------------------------------
// v0.0.2: Initial: an empty window titled Chrono Calc

namespace chrono_calc {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
};

}  // namespace chrono_calc
