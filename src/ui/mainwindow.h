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
// src/ui/mainwindow.h 0.4.11

// --- Version history ----------------------------------------------------
// v0.4.11: The Statistics view: its size, the list of values with its selection buttons, the count (D-023)
// v0.3.8: The Programmer view: its size, the bases and word sizes, the bit display, the digits each base allows
// v0.2.7: The Scientific view; View > Standard and Scientific switch the buttons and the window size; the view is
//         remembered; Inv labels, the toggles, the angle units and the bracket counter follow the calculator (D-020)
// v0.1.6: The Standard view: the menus, the display, the buttons and the keys, a fixed size and the position
//         remembered (tasks 1.1 to 1.4, D-018)
// v0.0.2: Initial: an empty window titled Chrono Calc

namespace chrono_calc {

class BitDisplay;
class BracketCounter;
class ChoiceButton;
class CalcButton;
class DisplayPanel;
class StatisticsList;

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
    // Turns a bit of the bit display over (the command "bit.N")
    void runBitCommand(int bit);
    // Runs a command that came from the keyboard and shows its button pressed for a moment
    void runKeyCommand(std::string_view commandId);
    // A calculator button was clicked: it runs its command, and the keyboard focus goes back from the list to the
    // calculator (Windows 7)
    void runButtonCommand(std::string_view commandId);
    // The Statistics view's list selects this value (a click, Home, End, Page Up, Page Down)
    void selectValue(int index);
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
    std::vector<ChoiceButton*> m_choiceButtons;
    BracketCounter* m_bracketCounter = nullptr;
    BitDisplay* m_bitDisplay = nullptr;
    StatisticsList* m_statisticsList = nullptr;
    std::vector<QAction*> m_viewActions;
};

}  // namespace chrono_calc
