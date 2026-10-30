#include <QApplication>
#include <QCoreApplication>
#include <QString>

#include "ui/look.h"
#include "ui/mainwindow.h"

// --- Version ------------------------------------------------------------
// src/app/main.cpp 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: The program's own look; the settings live in ~/.config/chrono-calc/
// v0.0.2: Initial: starts Qt and shows the main window

int main(int argc, char* argv[])
{
    const QApplication application(argc, argv);
    // The settings (the window's position) are kept in ~/.config/chrono-calc/chrono-calc.conf
    QCoreApplication::setOrganizationName(QStringLiteral("chrono-calc"));
    QCoreApplication::setApplicationName(QStringLiteral("chrono-calc"));
    chrono_calc::applyLook();
    chrono_calc::MainWindow window;
    window.show();
    return QApplication::exec();
}
