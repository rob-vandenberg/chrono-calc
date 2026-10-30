#include <QApplication>

#include "ui/mainwindow.h"

// --- Version ------------------------------------------------------------
// src/app/main.cpp 0.0.2

// --- Version history ----------------------------------------------------
// v0.0.2: Initial: starts Qt and shows the main window

int main(int argc, char* argv[])
{
    const QApplication application(argc, argv);
    chrono_calc::MainWindow window;
    window.show();
    return QApplication::exec();
}
