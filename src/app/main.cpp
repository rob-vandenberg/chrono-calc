#include <QApplication>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QString>

#include "ui/look.h"
#include "ui/mainwindow.h"

// --- Version ------------------------------------------------------------
// src/app/main.cpp 0.9.14

// --- Version history ----------------------------------------------------
// v0.9.14: The desktop entry's name, so a Wayland desktop shows the program's icon and name with the window
// v0.1.6: The program's own look; the settings live in ~/.config/chrono-calc/
// v0.0.2: Initial: starts Qt and shows the main window

int main(int argc, char* argv[])
{
    const QApplication application(argc, argv);
    // The settings (the window's position) are kept in ~/.config/chrono-calc/chrono-calc.conf
    QCoreApplication::setOrganizationName(QStringLiteral("chrono-calc"));
    QCoreApplication::setApplicationName(QStringLiteral("chrono-calc"));
    // The installed desktop entry (packaging/chrono-calc.desktop): the desktop takes the icon and the name "Calc" from it
    QGuiApplication::setDesktopFileName(QStringLiteral("chrono-calc"));
    chrono_calc::applyLook();
    chrono_calc::MainWindow window;
    window.show();
    return QApplication::exec();
}
