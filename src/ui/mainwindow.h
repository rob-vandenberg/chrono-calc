#pragma once

#include <QMainWindow>

#include "core/calculator.h"
#include "core/enginetexts.h"
#include "core/menus.h"
#include "core/view.h"

#include <map>
#include <string_view>
#include <vector>

class QAction;
class QActionGroup;
class QCloseEvent;
class QKeyEvent;
class QMenu;

// --- Version ------------------------------------------------------------
// src/ui/mainwindow.h 0.2.7

// --- Version history ----------------------------------------------------
// v0.2.7: The Scientific view; View > Standard and Scientific switch the buttons and the window size; the view is
//         remembered; Inv labels, the toggles, the angle units and the bracket counter follow the calculator (D-020)
// v0.1.6: The Standard view: the menus, the display, the buttons and the keys, a fixed size and the position
//         remembered (tasks 1.1 to 1.4, D-018)
// v0.0.2: Initial: an empty window titled Chrono Calc

namespace chrono_calc {

class AngleButton;
class BracketCounter;
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
    // Puts the display and the buttons of this view in place, and gives the window its size
    void buildView(View view);
    void runCommand(std::string_view commandId);
    // Runs a command that came from the keyboard and shows its button pressed for a moment
    void runKeyCommand(std::string_view commandId);
    void showCalculator();
    void restorePosition();
    void savePosition() const;

    Calculator m_calculator;
    Separators m_separators;
    QWidget* m_area = nullptr;
    DisplayPanel* m_display = nullptr;
    View m_builtView = View::Standard;
    // Everything of the current view (buttons, angle units, counter), replaced on a view change
    std::vector<QWidget*> m_viewParts;
    std::vector<CalcButton*> m_buttons;
    std::vector<AngleButton*> m_angleButtons;
    BracketCounter* m_bracketCounter = nullptr;
    std::vector<QAction*> m_viewActions;
};

}  // namespace chrono_calc
