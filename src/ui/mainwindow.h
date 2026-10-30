#pragma once

#include <QMainWindow>

#include "core/calculator.h"
#include "core/enginetexts.h"
#include "core/layout.h"
#include "core/menus.h"
#include "core/view.h"

#include <map>
#include <optional>
#include <string_view>
#include <vector>

class QAction;
class QActionGroup;
class QCloseEvent;
class QKeyEvent;
class QMenu;

// --- Version ------------------------------------------------------------
// src/ui/mainwindow.h 0.5.12

// --- Version history ----------------------------------------------------
// v0.5.12: History (the list in a taller display, the edit box), Digit grouping, Copy and Paste with the clipboard,
//          the Edit menu of each view, History and Digit grouping remembered (D-024)
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
class ListEditBox;
class StatisticsList;

// The calculator's window. Every key, click and menu item runs a command of the core and then shows what the
// calculator shows; the window computes nothing itself (AGENTS.md 4.1, 4.3).
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // The separators of the computer's regional settings
    explicit MainWindow(QWidget* parent = nullptr);
    MainWindow(const Separators& separators, QWidget* parent);

    // The separators and the grouping pattern of the computer's regional settings (the default QLocale)
    static Separators systemSeparators();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void buildMenus();
    // The Edit menu of the current view (Windows 7 has one per view), built again on a view change
    void buildEditMenu();
    void addMenuItems(QMenu* menu, const std::vector<MenuItem>& items);
    void addMenuEntry(QMenu* menu, const MenuEntry& entry, std::map<int, QActionGroup*>& dotGroups);
    // A built menu item runs its command; the items whose state follows the calculator are kept
    void connectMenuAction(QAction* action, std::string_view commandId);
    void buildLists(const ViewLayout& layout);
    void buildHistoryList(const ViewLayout& layout);
    void showMenuStates();
    void showLists();
    // The command of a key in the current view, as Windows 7 reads its keys
    [[nodiscard]] std::optional<std::string_view> commandOfKey(const QKeyEvent& event, Qt::KeyboardModifiers modifiers) const;
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
    // The history list chooses this entry (a click, Home, End)
    void chooseEntry(int index);
    // The keys of the history list that give the keyboard focus back to the calculator (Windows 7)
    void runListCommand(std::string_view commandId);
    // The edit box of the history or the Statistics list: opened where the calculator edits an entry, closed otherwise
    void showEditBox();
    void closeEditBox();
    void takeEdit(const QString& text);
    void dropEdit();
    // The text of the clipboard, read as typed keys
    void paste();
    void updatePaste();
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
    // The history list (Standard and Scientific with History shown), and the edit box over a row of a list
    StatisticsList* m_historyList = nullptr;
    ListEditBox* m_editBox = nullptr;
    // The list the edit box belongs to
    StatisticsList* m_editList = nullptr;
    bool m_builtHistory = false;
    std::vector<QAction*> m_viewActions;
    QMenu* m_editMenu = nullptr;
    // The menu items whose state follows the calculator: History and Digit grouping (ticks), Paste (greyed without text
    // on the clipboard), Edit (Recalculate while editing)
    QAction* m_historyAction = nullptr;
    QAction* m_groupingAction = nullptr;
    QAction* m_pasteAction = nullptr;
    QAction* m_editAction = nullptr;
};

}  // namespace chrono_calc
