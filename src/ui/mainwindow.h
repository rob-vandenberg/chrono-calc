#pragma once

#include <QMainWindow>

#include "core/calculator.h"
#include "core/enginetexts.h"
#include "core/menus.h"

#include <map>
#include <string_view>
#include <vector>

class QActionGroup;
class QCloseEvent;
class QKeyEvent;
class QMenu;

// --- Version ------------------------------------------------------------
// src/ui/mainwindow.h 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: The Standard view: the menus, the display, the buttons and the keys, a fixed size and the position
//         remembered (tasks 1.1 to 1.4, D-018)
// v0.0.2: Initial: an empty window titled Chrono Calc

namespace chrono_calc {

class CalcButton;
class DisplayPanel;

// The calculator's window. Every key, click and menu item runs a command of the core and then shows what the
// calculator shows; the window computes nothing itself (AGENTS.md 4.1, 4.3).
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // The separators of the computer's regional settings
    explicit MainWindow(QWidget* parent = nullptr);
    MainWindow(Separators separators, QWidget* parent);

    // The separators of the computer's regional settings (QLocale::system())
    static Separators systemSeparators();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void buildMenus();
    void addMenuEntry(QMenu* menu, const MenuEntry& entry, std::map<int, QActionGroup*>& dotGroups);
    void buildStandardView(Separators separators);
    void runCommand(std::string_view commandId);
    // Runs a command that came from the keyboard and shows its button pressed for a moment
    void runKeyCommand(std::string_view commandId);
    void showCalculator();
    void restorePosition();
    void savePosition() const;

    Calculator m_calculator;
    DisplayPanel* m_display = nullptr;
    std::vector<CalcButton*> m_buttons;
};

}  // namespace chrono_calc
