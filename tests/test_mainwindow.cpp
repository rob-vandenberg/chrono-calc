#include <QAction>
#include <QClipboard>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDesktopServices>
#include <QDialog>
#include <QEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QLocale>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QPoint>
#include <QRadioButton>
#include <QRect>
#include <QScreen>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <QWidget>

#include "core/enginetexts.h"
#include "core/version.h"
#include "ui/calcbutton.h"
#include "ui/displaypanel.h"
#include "ui/mainwindow.h"
#include "ui/statisticsparts.h"
#include "ui/viewparts.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>
#include <vector>

// --- Version ------------------------------------------------------------
// tests/test_mainwindow.cpp 0.9.14

// --- Version history ----------------------------------------------------
// v0.9.14: Edit > History greyed while the history is empty; Help > View Help opens the project's page, About Chrono
//          Calc (F1) the About box; the window's icon
// v0.8.13: The Unit conversion panel: beside every view, its sizes, converting, its keys, Tab order, remembered; the Date
//          calculation panel: beside every view, both modes, Calculate, its keys, Tab order, the focus, remembered; the
//          Worksheets: beside every view, their boxes and labels, Calculate, the messages, the list's keys, Tab order,
//          remembered
// v0.5.12: History (the taller window, the list, its keys, the edit box), Digit grouping, Copy and Paste with the
//          clipboard, the Edit menu of each view, History and Digit grouping remembered
// v0.4.11: The Statistics view: its size, the list, the count, the selection buttons, the keyboard focus of the list
// v0.3.10: Greek and Hebrew: the Q key is x² in Scientific; in Standard its sign counts
// v0.3.9: A Russian keyboard keeps the letter keys and Ctrl letters; AZERTY keeps its own letters
// v0.3.8: The Programmer view: its size, the bases, the bit display and its clicks, the greyed digits
// v0.2.7: The Scientific view: switching, its size, keys, Inv labels, toggles, angle units, bracket counter, the
//         remembered view; Standard's button faces
// v0.1.6: The Standard view (keys typed into the view area, as the real keyboard focus does): the measures, the buttons, the keys, the menus, the fixed size and the remembered position
// v0.0.2: Initial: the window opens with the title Chrono Calc

namespace {

// A comma as the decimal point, as on the owner's Windows computer that gave the reference results
constexpr chrono_calc::Separators commaSeparators{.decimal = L',', .thousands = L'.'};

const chrono_calc::DisplayPanel* displayOf(const chrono_calc::MainWindow& window)
{
    return window.findChild<chrono_calc::DisplayPanel*>(QStringLiteral("display"));
}

chrono_calc::CalcButton* buttonOf(const chrono_calc::MainWindow& window, const QString& commandId)
{
    return window.findChild<chrono_calc::CalcButton*>(QStringLiteral("button.") + commandId);
}

// The menu item with this text (the shortcut a greyed item shows after a tab is left out)
QAction* actionOf(const QMenu* menu, const QString& text)
{
    const QList<QAction*> actions = menu->actions();
    const auto found = std::ranges::find_if(actions, [&text](const QAction* action) {
        return action->text().section(QLatin1Char('\t'), 0, 0) == text;
    });
    return found == actions.end() ? nullptr : *found;
}

const QMenu* menuOf(const chrono_calc::MainWindow& window, const QString& title)
{
    for (const QAction* menuAction : window.menuBar()->actions()) {
        if (menuAction->text() == title) {
            return menuAction->menu();
        }
    }
    return nullptr;
}

// An item of View > Worksheets
QAction* worksheetAction(const chrono_calc::MainWindow& window, const QString& text)
{
    const QAction* submenu = actionOf(menuOf(window, QStringLiteral("&View")), QStringLiteral("&Worksheets"));
    return submenu == nullptr || submenu->menu() == nullptr ? nullptr : actionOf(submenu->menu(), text);
}

// A part of the window by its name. The parts a view or panel change replaces go once the event loop runs
// (MainWindow::buildView), so they are let go first: a replaced part is never found.
template <typename Part>
Part* partOf(const chrono_calc::MainWindow& window, const QString& name)
{
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    return window.findChild<Part*>(name);
}

// The settings go to a folder of their own that is removed afterwards: the test leaves nothing behind
std::unique_ptr<QTemporaryDir>& settingsFolder()
{
    static std::unique_ptr<QTemporaryDir> folder;
    return folder;
}

std::unique_ptr<chrono_calc::MainWindow> openWindow()
{
    auto window = std::make_unique<chrono_calc::MainWindow>(commaSeparators, nullptr);
    window->show();
    if (!QTest::qWaitForWindowExposed(window.get())) {
        return nullptr;
    }
    return window;
}

// A key pressed and let go as the keyboard of Linux gives it: Qt's key, the typed text, the XKB key code, which is
// the PC scan code plus 8, and the layout's key symbol (0: none given)
void pressKey(QWidget* widget, int key, const QString& text, std::uint32_t scanCode,
              Qt::KeyboardModifiers modifiers = Qt::NoModifier, std::uint32_t keySymbol = 0)
{
    constexpr std::uint32_t xkbOffset = 8;
    QKeyEvent press(QEvent::KeyPress, key, modifiers, scanCode + xkbOffset, keySymbol, 0, text);
    QCoreApplication::sendEvent(widget, &press);
    QKeyEvent release(QEvent::KeyRelease, key, modifiers, scanCode + xkbOffset, keySymbol, 0, text);
    QCoreApplication::sendEvent(widget, &release);
}

// Receives the addresses Help > View Help opens, instead of the browser
class UrlCatcher : public QObject
{
    Q_OBJECT

public:
    [[nodiscard]] const QStringList& opened() const { return m_opened; }

public slots:  // NOLINT(readability-redundant-access-specifiers): Qt's slots section
    void open(const QUrl& url) { m_opened.append(url.toString()); }

private:
    QStringList m_opened;
};

}  // namespace

class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    static void initTestCase()
    {
        settingsFolder() = std::make_unique<QTemporaryDir>();
        // The keyboard layouts the window finds without a desktop (XKB's defaults, with the offscreen platform CTest
        // gives the tests): US, Greek, Hebrew and French
        // (AZERTY), and nothing else of the developer's own XKB settings
        qputenv("XKB_DEFAULT_LAYOUT", "us,gr,il,fr");
        for (const char* name : {"XKB_DEFAULT_RULES", "XKB_DEFAULT_MODEL", "XKB_DEFAULT_VARIANT", "XKB_DEFAULT_OPTIONS"}) {
            qunsetenv(name);
        }
        QVERIFY(settingsFolder()->isValid());
        QCoreApplication::setOrganizationName(QStringLiteral("chrono-calc-test"));
        QCoreApplication::setApplicationName(QStringLiteral("test_mainwindow"));
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, settingsFolder()->path());
    }

    static void init()
    {
        QSettings().clear();
    }

    static void cleanupTestCase()
    {
        settingsFolder().reset();
    }

    static void theWindowOpensWithTheTitleChronoCalc()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QCOMPARE(window->windowTitle(), QStringLiteral("Chrono Calc"));
    }

    // The client area below the menus is Windows 7's 264 x 331, and the window cannot be resized
    static void theWindowHasTheFixedSizeOfTheStandardView()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QCOMPARE(window->centralWidget()->size(), QSize(264, 331));
        QCOMPARE(window->minimumSize(), window->maximumSize());
    }

    // Every measure of tests/reference/windows7-standard.md part A
    static void theDisplayAndTheButtonsSitWhereWindows7HasThem()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QCOMPARE(displayOf(*window)->geometry(), QRect(13, 13, 238, 63));
        QCOMPARE(window->findChildren<chrono_calc::CalcButton*>().size(), 28);
        QCOMPARE(buttonOf(*window, QStringLiteral("memory.clear"))->geometry(), QRect(13, 82, 43, 33));
        QCOMPARE(buttonOf(*window, QStringLiteral("sqrt"))->geometry(), QRect(208, 122, 43, 33));
        QCOMPARE(buttonOf(*window, QStringLiteral("percent"))->geometry(), QRect(208, 162, 43, 33));
        QCOMPARE(buttonOf(*window, QStringLiteral("digit.6"))->geometry(), QRect(111, 203, 43, 33));
        QCOMPARE(buttonOf(*window, QStringLiteral("equals"))->geometry(), QRect(208, 242, 43, 74));
        QCOMPARE(buttonOf(*window, QStringLiteral("digit.0"))->geometry(), QRect(13, 282, 92, 33));
        QCOMPARE(buttonOf(*window, QStringLiteral("decimal"))->geometry(), QRect(111, 282, 43, 33));
        QCOMPARE(buttonOf(*window, QStringLiteral("add"))->geometry(), QRect(160, 282, 43, 33));
    }

    // The decimal-point button shows the regional separator
    static void theDecimalButtonShowsTheRegionalSeparator()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QCOMPARE(buttonOf(*window, QStringLiteral("decimal"))->text(), QStringLiteral(","));
        const chrono_calc::MainWindow pointWindow(chrono_calc::Separators{.decimal = L'.', .thousands = L','}, nullptr);
        QCOMPARE(buttonOf(pointWindow, QStringLiteral("decimal"))->text(), QStringLiteral("."));
    }

    // Clicks run the commands and the display shows the calculator: result 1 of the reference
    static void clickingTheButtonsCalculates()
    {
        const auto window = openWindow();
        QVERIFY(window);
        for (const QString& command : {QStringLiteral("digit.2"), QStringLiteral("add"), QStringLiteral("digit.3"), QStringLiteral("multiply")}) {
            QTest::mouseClick(buttonOf(*window, command), Qt::LeftButton);
        }
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("5"));
        QCOMPARE(displayOf(*window)->expression(), QStringLiteral("2 + 3 *"));
    }

    // Typed characters, as in Windows 7: 0,1+0.2= gives 0,3 (both , and . are the decimal point)
    static void typingCalculates()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("0,1+0.2="));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0,3"));
        QTest::keyClicks(window->centralWidget(), QStringLiteral("9@"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("3"));
        QTest::keyClicks(window->centralWidget(), QStringLiteral("5r"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0,2"));
        QTest::keyClicks(window->centralWidget(), QStringLiteral("2+3%"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0,06"));
    }

    // The keys without a character: Escape = C, Delete = CE, Backspace, Enter = equals, F9 = plus/minus
    static void theKeysWithoutACharacterRunTheirCommands()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("123"));
        QTest::keyClick(window->centralWidget(), Qt::Key_Backspace);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("12"));
        QTest::keyClicks(window->centralWidget(), QStringLiteral("+3"));
        QTest::keyClick(window->centralWidget(), Qt::Key_Delete);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));
        QTest::keyClicks(window->centralWidget(), QStringLiteral("4"));
        QTest::keyClick(window->centralWidget(), Qt::Key_Return);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("16"));
        QTest::keyClick(window->centralWidget(), Qt::Key_F9);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("-16"));
        QTest::keyClicks(window->centralWidget(), QStringLiteral("2*3"));
        QTest::keyClick(window->centralWidget(), Qt::Key_Enter, Qt::KeypadModifier);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("6"));
        QTest::keyClicks(window->centralWidget(), QStringLiteral("7+"));
        QTest::keyClick(window->centralWidget(), Qt::Key_Escape);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));
        QCOMPARE(displayOf(*window)->expression(), QString());
    }

    // The memory keys Ctrl+M (MS), Ctrl+R (MR), Ctrl+P (M+), Ctrl+Q (M-), Ctrl+L (MC), and the mark M
    static void theCtrlKeysRunTheMemory()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("5"));
        QTest::keyClick(window->centralWidget(), Qt::Key_M, Qt::ControlModifier);
        QVERIFY(displayOf(*window)->memoryMarkShown());
        QTest::keyClick(window->centralWidget(), Qt::Key_Escape);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("3"));
        QTest::keyClick(window->centralWidget(), Qt::Key_P, Qt::ControlModifier);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("1"));
        QTest::keyClick(window->centralWidget(), Qt::Key_Q, Qt::ControlModifier);
        QTest::keyClick(window->centralWidget(), Qt::Key_R, Qt::ControlModifier);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("7"));
        QTest::keyClick(window->centralWidget(), Qt::Key_L, Qt::ControlModifier);
        QVERIFY(!displayOf(*window)->memoryMarkShown());
    }

    // On a Russian keyboard the letter keys keep their commands by position, as in Windows 7 (reference part F):
    // Ctrl+M (types ь) stores, S (ы) is sin, A and B (ф и) are hexadecimal digits; AZERTY keeps its own letters
    static void aKeyboardWithoutLatinLettersKeepsTheLetterKeys()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QWidget* view = window->centralWidget();
        QTest::keyClicks(view, QStringLiteral("7"));
        pressKey(view, 0x42C, QString(), 0x32, Qt::ControlModifier);
        QVERIFY(displayOf(*window)->memoryMarkShown());
        pressKey(view, 0x414, QString(), 0x26, Qt::ControlModifier);
        QVERIFY(!displayOf(*window)->memoryMarkShown());
        QTest::keyClick(view, Qt::Key_2, Qt::AltModifier);
        QTest::keyClicks(view, QStringLiteral("30"));
        pressKey(view, 0x42B, QStringLiteral("\u044B"), 0x1F);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0,5"));
        QCOMPARE(displayOf(*window)->expression(), QStringLiteral("sind(30)"));
        // German and Russian, Russian in use: Ctrl+я comes as Ctrl+Y (Qt takes German's letter) with Cyrillic's key
        // symbol; Windows 7 reads the key's place, Z: nothing, not the y-th root (Ctrl+Y)
        QTest::keyClick(view, Qt::Key_Escape);
        QTest::keyClicks(view, QStringLiteral("8"));
        pressKey(view, Qt::Key_Y, QString(), 0x2C, Qt::ControlModifier, 0x6D1);
        QCOMPARE(displayOf(*window)->expression(), QString());
        pressKey(view, Qt::Key_Y, QString(), 0x15, Qt::ControlModifier, 0x6CE);
        QCOMPARE(displayOf(*window)->expression(), QStringLiteral("8 yroot"));
        QTest::keyClick(view, Qt::Key_3, Qt::AltModifier);
        QTest::keyClick(view, Qt::Key_F5);
        pressKey(view, 0x424, QStringLiteral("\u0444"), 0x1E);
        pressKey(view, 0x418, QStringLiteral("\u0438"), 0x30);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("AB"));
        // A Russian key off the 26 letter places (х) and a key without its place give nothing
        pressKey(view, 0x425, QStringLiteral("\u0445"), 0x1A);
        pressKey(view, 0x421, QStringLiteral("\u0441"), 0);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("AB"));
        // Thai: the A key (\u0E1F) is A, the J key, which types a mark (\u0E48), is RoL: A becomes 14
        QTest::keyClick(view, Qt::Key_Escape);
        pressKey(view, 0xE1F, QStringLiteral("\u0E1F"), 0x1E);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("A"));
        pressKey(view, 0xE48, QStringLiteral("\u0E48"), 0x24);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("14"));
        // AZERTY: the key at the place of the US Q types a and stays A; Ctrl+M of AZERTY (at the US ; place) stores
        QTest::keyClick(view, Qt::Key_Escape);
        pressKey(view, Qt::Key_A, QStringLiteral("a"), 0x10);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("A"));
        pressKey(view, Qt::Key_M, QString(), 0x27, Qt::ControlModifier);
        QVERIFY(displayOf(*window)->memoryMarkShown());
        QTest::keyClick(view, Qt::Key_L, Qt::ControlModifier);
    }

    // Greek and Hebrew (Windows 7, reference part F line 6): the Q key types ; or / ; where the view has a command for
    // Q (x² in Scientific) that wins, elsewhere the sign counts (; is nothing in Standard, / divides)
    static void aSignOnALetterPlaceCountsOnlyWhereTheLetterHasNoCommand()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QWidget* view = window->centralWidget();
        constexpr std::uint32_t qPlace = 0x10;
        QTest::keyClicks(view, QStringLiteral("8"));
        pressKey(view, Qt::Key_Semicolon, QStringLiteral(";"), qPlace, Qt::NoModifier, 0x3B);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("8"));
        QCOMPARE(displayOf(*window)->expression(), QString());
        pressKey(view, Qt::Key_Slash, QStringLiteral("/"), qPlace, Qt::NoModifier, 0x2F);
        QCOMPARE(displayOf(*window)->expression(), QStringLiteral("8 /"));
        QTest::keyClicks(view, QStringLiteral("2"));
        QTest::keyClick(view, Qt::Key_Return);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("4"));
        QTest::keyClick(view, Qt::Key_2, Qt::AltModifier);
        QTest::keyClicks(view, QStringLiteral("3"));
        pressKey(view, Qt::Key_Semicolon, QStringLiteral(";"), qPlace, Qt::NoModifier, 0x3B);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("9"));
        QCOMPARE(displayOf(*window)->expression(), QStringLiteral("sqr(3)"));
        QTest::keyClick(view, Qt::Key_Escape);
        QTest::keyClicks(view, QStringLiteral("3"));
        pressKey(view, Qt::Key_Slash, QStringLiteral("/"), qPlace, Qt::NoModifier, 0x2F);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("9"));
        QCOMPARE(displayOf(*window)->expression(), QStringLiteral("sqr(3)"));
        // AZERTY's , on the place of the US M is the decimal point in Standard, not M
        QTest::keyClick(view, Qt::Key_1, Qt::AltModifier);
        QTest::keyClicks(view, QStringLiteral("1"));
        pressKey(view, Qt::Key_Comma, QStringLiteral(","), 0x32, Qt::NoModifier, 0x2C);
        QTest::keyClicks(view, QStringLiteral("5"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("1,5"));
        QTest::keyClick(view, Qt::Key_2, Qt::AltModifier);
        // The US ; key, off the 26 letter places, is still Int
        QTest::keyClick(view, Qt::Key_Escape);
        QTest::keyClicks(view, QStringLiteral("3"));
        pressKey(view, Qt::Key_Semicolon, QStringLiteral(";"), 0x27, Qt::NoModifier, 0x3B);
        QCOMPARE(displayOf(*window)->expression(), QStringLiteral("Int(3)"));
    }

    // A typed key shows its button pressed for a moment
    static void aTypedKeyShowsItsButtonPressed()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("7"));
        QVERIFY(buttonOf(*window, QStringLiteral("digit.7"))->looksPressed());
        QTRY_VERIFY(!buttonOf(*window, QStringLiteral("digit.7"))->looksPressed());
    }

    // A long number is drawn smaller, so that every digit stays in view
    static void aLongNumberIsDrawnSmaller()
    {
        const auto window = openWindow();
        QVERIFY(window);
        const int normal = displayOf(*window)->numberPixelSize();
        QTest::keyClicks(window->centralWidget(), QStringLiteral("9999999999999999*9999999999999999="));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("9,999999999999998e+31"));
        QVERIFY(displayOf(*window)->numberPixelSize() < normal);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("1/0="));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("Cannot divide by zero"));
    }

    // The menus of Windows 7, every item present; what is not built yet is greyed, never hidden
    static void theMenusHaveEveryItemAndGreyWhatIsNotBuilt()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QStringList titles;
        for (const QAction* action : window->menuBar()->actions()) {
            titles << action->text();
        }
        QCOMPARE(titles, QStringList({QStringLiteral("&View"), QStringLiteral("&Edit"), QStringLiteral("&Help")}));

        const QMenu* view = menuOf(*window, QStringLiteral("&View"));
        QVERIFY(view != nullptr);
        const QAction* standard = actionOf(view, QStringLiteral("S&tandard"));
        QVERIFY(standard != nullptr && standard->isEnabled() && standard->isChecked());
        QCOMPARE(standard->shortcut(), QKeySequence(QStringLiteral("Alt+1")));
        const QAction* basic = actionOf(view, QStringLiteral("&Basic"));
        QVERIFY(basic != nullptr && basic->isEnabled() && basic->isChecked());
        const QAction* scientificItem = actionOf(view, QStringLiteral("&Scientific"));
        QVERIFY(scientificItem != nullptr && scientificItem->isEnabled() && !scientificItem->isChecked());
        QCOMPARE(scientificItem->shortcut(), QKeySequence(QStringLiteral("Alt+2")));
        QVERIFY(actionOf(view, QStringLiteral("&Programmer"))->isEnabled());
        QVERIFY(actionOf(view, QStringLiteral("St&atistics"))->isEnabled());
        QCOMPARE(actionOf(view, QStringLiteral("St&atistics"))->shortcut(), QKeySequence(QStringLiteral("Alt+4")));
        for (const QString& panel : {QStringLiteral("&Unit conversion"), QStringLiteral("&Date calculation")}) {
            const QAction* action = actionOf(view, panel);
            QVERIFY2(action != nullptr && action->isEnabled() && !action->isChecked(), qPrintable(panel));
        }
        for (const QString& ticked : {QStringLiteral("Histor&y"), QStringLiteral("D&igit grouping")}) {
            const QAction* action = actionOf(view, ticked);
            QVERIFY2(action != nullptr && action->isEnabled() && action->isCheckable() && !action->isChecked(), qPrintable(ticked));
        }
        QCOMPARE(actionOf(view, QStringLiteral("Histor&y"))->shortcut(), QKeySequence(QStringLiteral("Ctrl+H")));
        const QAction* worksheets = actionOf(view, QStringLiteral("&Worksheets"));
        QVERIFY(worksheets != nullptr && worksheets->menu() != nullptr);
        QCOMPARE(worksheets->menu()->actions().size(), 4);

        const QMenu* edit = menuOf(*window, QStringLiteral("&Edit"));
        QVERIFY(edit != nullptr);
        QVERIFY(actionOf(edit, QStringLiteral("&Copy"))->isEnabled());
        QCOMPARE(actionOf(edit, QStringLiteral("&Copy"))->shortcut(), QKeySequence(QStringLiteral("Ctrl+C")));
        QCOMPARE(actionOf(edit, QStringLiteral("&Paste"))->shortcut(), QKeySequence(QStringLiteral("Ctrl+V")));
        const QMenu* history = actionOf(edit, QStringLiteral("&History"))->menu();
        QCOMPARE(history->actions().size(), 4);
        // The history is empty: all four are greyed
        for (const QAction* action : history->actions()) {
            QVERIFY(!action->isEnabled());
        }
        // F2 and Esc are only shown: the window's keys do them (Esc is C, F2 a word size in Programmer)
        QVERIFY(actionOf(history, QStringLiteral("&Edit"))->shortcut().isEmpty());
        QCOMPARE(actionOf(history, QStringLiteral("&Edit"))->text(), QStringLiteral("&Edit\tF2"));
        QCOMPARE(actionOf(history, QStringLiteral("C&lear"))->shortcut(), QKeySequence(QStringLiteral("Ctrl+Shift+D")));

        const QMenu* help = menuOf(*window, QStringLiteral("&Help"));
        QVERIFY(help != nullptr);
        QVERIFY(actionOf(help, QStringLiteral("&View Help"))->isEnabled());
        QVERIFY(actionOf(help, QStringLiteral("&View Help"))->shortcut().isEmpty());
        QVERIFY(actionOf(help, QStringLiteral("&About Chrono Calc"))->isEnabled());
        QCOMPARE(actionOf(help, QStringLiteral("&About Chrono Calc"))->shortcut(), QKeySequence(QStringLiteral("F1")));
        QVERIFY(!window->windowIcon().isNull());
    }

    // View > Scientific: the window takes Windows 7's Scientific size, the number is cleared, the memory kept, the dot moves
    static void scientificSwitchesTheViewAndTheSize()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("5"));
        QTest::keyClick(window->centralWidget(), Qt::Key_M, Qt::ControlModifier);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("12+3"));
        const QMenu* view = menuOf(*window, QStringLiteral("&View"));
        actionOf(view, QStringLiteral("&Scientific"))->trigger();
        QCOMPARE(window->centralWidget()->size(), QSize(508, 331));
        QCOMPARE(displayOf(*window)->geometry(), QRect(13, 13, 482, 63));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));
        QCOMPARE(displayOf(*window)->expression(), QString());
        QVERIFY(displayOf(*window)->memoryMarkShown());
        QVERIFY(actionOf(view, QStringLiteral("&Scientific"))->isChecked());
        QVERIFY(!actionOf(view, QStringLiteral("S&tandard"))->isChecked());
        QTRY_COMPARE(window->findChildren<chrono_calc::CalcButton*>().size(), 52);
        QCOMPARE(buttonOf(*window, QStringLiteral("power_of_ten"))->geometry(), QRect(208, 282, 43, 33));
        QVERIFY(!buttonOf(*window, QStringLiteral("percent"))->isEnabled());
        actionOf(view, QStringLiteral("S&tandard"))->trigger();
        QCOMPARE(window->centralWidget()->size(), QSize(264, 331));
        QTRY_COMPARE(window->findChildren<chrono_calc::CalcButton*>().size(), 28);
        QVERIFY(buttonOf(*window, QStringLiteral("percent"))->isEnabled());
    }

    // The Scientific keys: letters for the functions, F4 for Radians, Ctrl+G for 10^x, ( ) and the bracket counter
    static void theScientificKeysRunTheirCommands()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClick(window->centralWidget(), Qt::Key_2, Qt::AltModifier);
        QCOMPARE(window->centralWidget()->size(), QSize(508, 331));
        QTest::keyClicks(window->centralWidget(), QStringLiteral("30s"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0,5"));
        QCOMPARE(displayOf(*window)->expression(), QStringLiteral("sind(30)"));
        // With Caps Lock on the S key types "S" and still gives sin; Shift+S gives nothing (Windows 7)
        QTest::keyClick(window->centralWidget(), Qt::Key_Escape);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("30"));
        QTest::sendKeyEvent(QTest::Click, window->centralWidget(), Qt::Key_S, QStringLiteral("S"), Qt::NoModifier);
        QCOMPARE(displayOf(*window)->expression(), QStringLiteral("sind(30)"));
        QTest::keyClick(window->centralWidget(), Qt::Key_Escape);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("30"));
        QTest::sendKeyEvent(QTest::Click, window->centralWidget(), Qt::Key_S, QStringLiteral("S"), Qt::ShiftModifier);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("30"));
        // Shift+S with Caps Lock on types "s": still nothing
        QTest::sendKeyEvent(QTest::Click, window->centralWidget(), Qt::Key_S, QStringLiteral("s"), Qt::ShiftModifier);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("30"));
        QTest::keyClick(window->centralWidget(), Qt::Key_Escape);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("3"));
        QTest::keyClick(window->centralWidget(), Qt::Key_G, Qt::ControlModifier);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("1000"));
        QTest::keyClick(window->centralWidget(), Qt::Key_Escape);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("(2+3"));
        QCOMPARE(window->findChild<chrono_calc::BracketCounter*>(QStringLiteral("brackets"))->text(), QStringLiteral("(=1"));
        QTest::keyClicks(window->centralWidget(), QStringLiteral(")*4="));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("20"));
        QCOMPARE(window->findChild<chrono_calc::BracketCounter*>(QStringLiteral("brackets"))->text(), QString());
        QTest::keyClick(window->centralWidget(), Qt::Key_F4);
        QVERIFY(window->findChild<chrono_calc::ChoiceButton*>(QStringLiteral("choice.angle.radians"))->isChecked());
        QVERIFY(!window->findChild<chrono_calc::ChoiceButton*>(QStringLiteral("choice.angle.degrees"))->isChecked());
        QTest::keyClicks(window->centralWidget(), QStringLiteral("2+3%"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("3"));
    }

    // Inv: its button turns warm, the labels show the inverse forms, and it turns off after sin
    static void inverseChangesTheLabelsAndEndsAfterUse()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClick(window->centralWidget(), Qt::Key_2, Qt::AltModifier);
        QTRY_VERIFY(buttonOf(*window, QStringLiteral("inverse")) != nullptr);
        QTest::mouseClick(buttonOf(*window, QStringLiteral("inverse")), Qt::LeftButton);
        QVERIFY(buttonOf(*window, QStringLiteral("inverse"))->isOn());
        QCOMPARE(buttonOf(*window, QStringLiteral("sin"))->text(), QStringLiteral("sin\u207B\u00B9"));
        QCOMPARE(buttonOf(*window, QStringLiteral("ln"))->text(), QStringLiteral("e\u02E3"));
        QCOMPARE(buttonOf(*window, QStringLiteral("log"))->text(), QStringLiteral("log"));
        QTest::keyClicks(window->centralWidget(), QStringLiteral("1"));
        QTest::mouseClick(buttonOf(*window, QStringLiteral("sin")), Qt::LeftButton);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("90"));
        QVERIFY(!buttonOf(*window, QStringLiteral("inverse"))->isOn());
        QCOMPARE(buttonOf(*window, QStringLiteral("sin"))->text(), QStringLiteral("sin"));
        QTest::mouseClick(window->findChild<chrono_calc::ChoiceButton*>(QStringLiteral("choice.angle.grads")), Qt::LeftButton);
        QVERIFY(window->findChild<chrono_calc::ChoiceButton*>(QStringLiteral("choice.angle.grads"))->isChecked());
    }

    // View > Programmer: Windows 7's size, Dec and Qword chosen, the bits, greyed digits per base, a bit click
    static void theProgrammerViewWorksLikeWindows7()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClick(window->centralWidget(), Qt::Key_3, Qt::AltModifier);
        QCOMPARE(window->centralWidget()->size(), QSize(508, 413));
        QTRY_COMPARE(window->findChildren<chrono_calc::CalcButton*>().size(), 45);
        QVERIFY(window->findChild<chrono_calc::ChoiceButton*>(QStringLiteral("choice.base.decimal"))->isChecked());
        QVERIFY(window->findChild<chrono_calc::ChoiceButton*>(QStringLiteral("choice.word.qword"))->isChecked());
        QVERIFY(!buttonOf(*window, QStringLiteral("digit.a"))->isEnabled());
        QVERIFY(!buttonOf(*window, QStringLiteral("decimal"))->isEnabled());
        QTest::keyClicks(window->centralWidget(), QStringLiteral("255"));
        auto* bits = window->findChild<chrono_calc::BitDisplay*>(QStringLiteral("bits"));
        QCOMPARE(bits->bits(), std::string(56, '0') + std::string(8, '1'));
        QCOMPARE(bits->visibleBits(), 64);
        QTest::keyClick(window->centralWidget(), Qt::Key_F5);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("FF"));
        QVERIFY(buttonOf(*window, QStringLiteral("digit.a"))->isEnabled());
        // The hexadecimal digits come from the letter keys, with or without Shift (Windows 7)
        QTest::keyClick(window->centralWidget(), Qt::Key_Escape);
        QTest::keyClick(window->centralWidget(), Qt::Key_A);
        QTest::sendKeyEvent(QTest::Click, window->centralWidget(), Qt::Key_B, QStringLiteral("B"), Qt::ShiftModifier);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("AB"));
        QTest::keyClick(window->centralWidget(), Qt::Key_Escape);
        QTest::keyClick(window->centralWidget(), Qt::Key_F6);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("255"));
        QTest::keyClick(window->centralWidget(), Qt::Key_F5);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("FF"));
        QTest::keyClick(window->centralWidget(), Qt::Key_F4);
        QCOMPARE(bits->visibleBits(), 8);
        QTest::keyClick(window->centralWidget(), Qt::Key_F8);
        QVERIFY(!buttonOf(*window, QStringLiteral("digit.2"))->isEnabled());
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("11111111"));
        // A click on bit 0 turns it over: 1111 1110
        QTest::mouseClick(bits, Qt::LeftButton, Qt::NoModifier, bits->bitArea(0).center());
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("11111110"));
        QTest::keyClicks(window->centralWidget(), QStringLiteral("2"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("11111110"));
        // The base and the word size stay over a view change, the number does not
        QTest::keyClick(window->centralWidget(), Qt::Key_1, Qt::AltModifier);
        QTest::keyClick(window->centralWidget(), Qt::Key_3, Qt::AltModifier);
        QTRY_VERIFY(window->findChild<chrono_calc::ChoiceButton*>(QStringLiteral("choice.base.binary")) != nullptr);
        QVERIFY(window->findChild<chrono_calc::ChoiceButton*>(QStringLiteral("choice.base.binary"))->isChecked());
        QVERIFY(window->findChild<chrono_calc::ChoiceButton*>(QStringLiteral("choice.word.byte"))->isChecked());
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));
    }

    // View > Statistics (Alt+4): Windows 7's size, the list in the display with its count; Enter adds, the newest is
    // selected; a click on a value gives the list the keyboard focus, then the arrows move the selection and Delete
    // removes the selected value; a calculator button or Enter gives the focus back (windows7-statistics.md part D)
    static void theStatisticsViewWorksLikeWindows7()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClick(window->centralWidget(), Qt::Key_4, Qt::AltModifier);
        QCOMPARE(window->centralWidget()->size(), QSize(264, 456));
        QTRY_COMPARE(window->findChildren<chrono_calc::CalcButton*>().size(), 29);
        QVERIFY(actionOf(menuOf(*window, QStringLiteral("&View")), QStringLiteral("St&atistics"))->isChecked());
        auto* list = window->findChild<chrono_calc::StatisticsList*>(QStringLiteral("statistics.list"));
        QVERIFY(list != nullptr);
        QCOMPARE(displayOf(*window)->countText(), QStringLiteral("Count = 0"));
        for (const QString& value : {QStringLiteral("5"), QStringLiteral("3"), QStringLiteral("4"), QStringLiteral("6")}) {
            QTest::keyClicks(window->centralWidget(), value);
            QTest::keyClick(window->centralWidget(), Qt::Key_Return);
        }
        QCOMPARE(list->values(), QStringList({QStringLiteral("5"), QStringLiteral("3"), QStringLiteral("4"), QStringLiteral("6")}));
        QCOMPARE(list->selected(), 3);
        QCOMPARE(displayOf(*window)->countText(), QStringLiteral("Count = 4"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));
        // Delete without the list's focus does nothing (no CE in this view)
        QTest::keyClicks(window->centralWidget(), QStringLiteral("7"));
        QTest::keyClick(window->centralWidget(), Qt::Key_Delete);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("7"));
        QCOMPARE(list->values().size(), 4);
        // A click on the second value selects it and gives the list the focus; Delete removes it, the next one moves up
        // into the selection; the display keeps its number
        QTest::mouseClick(list, Qt::LeftButton, Qt::NoModifier, QPoint(list->width() / 2, 18 + 9));
        QCOMPARE(list->selected(), 1);
        QVERIFY(list->hasFocus());
        QTest::keyClick(list, Qt::Key_Delete);
        QCOMPARE(list->values(), QStringList({QStringLiteral("5"), QStringLiteral("4"), QStringLiteral("6")}));
        QCOMPARE(list->selected(), 1);
        QCOMPARE(displayOf(*window)->countText(), QStringLiteral("Count = 3"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("7"));
        QTest::keyClick(list, Qt::Key_Up);
        QCOMPARE(list->selected(), 0);
        QTest::keyClick(list, Qt::Key_Up);
        QCOMPARE(list->selected(), 0);
        QTest::keyClick(list, Qt::Key_End);
        QCOMPARE(list->selected(), 2);
        // Digits still go to the display while the list has the focus; Enter adds and gives the focus back
        QTest::keyClicks(list, QStringLiteral("2"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("72"));
        QVERIFY(list->hasFocus());
        QTest::keyClick(list, Qt::Key_Return);
        QVERIFY(!list->hasFocus());
        QCOMPARE(list->values().last(), QStringLiteral("72"));
        QCOMPARE(list->selected(), 3);
        // The selection buttons move the selection and give the list the focus; a calculator button takes it back
        QTest::mouseClick(window->findChild<chrono_calc::SelectionArrow*>(QStringLiteral("statistics.up")), Qt::LeftButton);
        QCOMPARE(list->selected(), 2);
        QVERIFY(list->hasFocus());
        QTest::mouseClick(buttonOf(*window, QStringLiteral("statistics.mean")), Qt::LeftButton);
        QVERIFY(!list->hasFocus());
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("21,75"));
        // The list is kept over a view change; the number is cleared
        QTest::keyClick(window->centralWidget(), Qt::Key_1, Qt::AltModifier);
        QTest::keyClick(window->centralWidget(), Qt::Key_4, Qt::AltModifier);
        // The parts of the previous view go once their own events are over
        QTRY_COMPARE(window->findChildren<chrono_calc::StatisticsList*>().size(), 1);
        list = window->findChild<chrono_calc::StatisticsList*>(QStringLiteral("statistics.list"));
        QCOMPARE(list->values().size(), 4);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));
        // D is CAD: the list is empty
        QTest::keyClick(window->centralWidget(), Qt::Key_D);
        QCOMPARE(list->values().size(), 0);
        QCOMPARE(displayOf(*window)->countText(), QStringLiteral("Count = 0"));
    }

    // View > History (windows7-history.md parts A, C and D): the window grows by 125 px and shows the list of entries;
    // the keys move the selection and show the entry's result; F2 opens the edit box over the entry, Enter
    // calculates it in place, Escape drops it; History is greyed in Programmer and keeps its tick; it is remembered
    static void historyShowsTheCalculations()
    {
        auto window = openWindow();
        QVERIFY(window);
        QWidget* area = window->centralWidget();
        QTest::keyClick(area, Qt::Key_H, Qt::ControlModifier);
        QCOMPARE(area->size(), QSize(264, 456));
        QVERIFY(actionOf(menuOf(*window, QStringLiteral("&View")), QStringLiteral("Histor&y"))->isChecked());
        auto* list = window->findChild<chrono_calc::StatisticsList*>(QStringLiteral("history.list"));
        QVERIFY(list != nullptr);
        QCOMPARE(list->geometry(), QRect(20, 46, 224, 91));
        // The buttons of the shorter layout go once their own events are over
        QTRY_COMPARE(buttonOf(*window, QStringLiteral("memory.clear"))->geometry().topLeft(), QPoint(13, 207));
        QTest::keyClicks(area, QStringLiteral("2+3=2*4="));
        QCOMPARE(list->values(), QStringList({QStringLiteral("2 + 3"), QStringLiteral("2 * 4")}));
        QCOMPARE(list->selected(), 1);
        // Up shows the result of the entry above
        QTest::keyClick(area, Qt::Key_Up);
        QCOMPARE(list->selected(), 0);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("5"));
        // A click chooses an entry and gives the list the focus; Up in the list gives it back
        QTest::mouseClick(list, Qt::LeftButton, Qt::NoModifier, QPoint(list->width() / 2, 18 + 9));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("8"));
        QVERIFY(list->hasFocus());
        QTest::keyClick(list, Qt::Key_Up);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("5"));
        QVERIFY(!list->hasFocus());
        // F2 edits the selected entry; Enter calculates it in place and the list gets the focus
        QTest::keyClick(area, Qt::Key_F2);
        auto* box = window->findChild<chrono_calc::ListEditBox*>(QStringLiteral("edit.box"));
        QVERIFY(box != nullptr);
        QCOMPARE(box->text(), QStringLiteral("2 + 3"));
        QVERIFY(box->hasFocus());
        const QAction* recalculate =
            actionOf(actionOf(menuOf(*window, QStringLiteral("&Edit")), QStringLiteral("&History"))->menu(), QStringLiteral("&Recalculate"));
        QVERIFY(recalculate != nullptr);
        QCOMPARE(recalculate->text(), QStringLiteral("&Recalculate\t="));
        box->selectAll();
        QTest::keyClicks(box, QStringLiteral("6*7"));
        QTest::keyClick(box, Qt::Key_Return);
        QTRY_VERIFY(window->findChild<chrono_calc::ListEditBox*>(QStringLiteral("edit.box")) == nullptr);
        QCOMPARE(list->values(), QStringList({QStringLiteral("6 * 7"), QStringLiteral("2 * 4")}));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("42"));
        QVERIFY(list->hasFocus());
        // An edit that cannot be read keeps the box open; Escape drops it
        QTest::keyClick(area, Qt::Key_F2);
        box = window->findChild<chrono_calc::ListEditBox*>(QStringLiteral("edit.box"));
        QVERIFY(box != nullptr);
        box->selectAll();
        QTest::keyClicks(box, QStringLiteral("2+*3"));
        QTest::keyClick(box, Qt::Key_Return);
        QVERIFY(box->isVisible());
        QTest::keyClick(box, Qt::Key_Escape);
        QTRY_VERIFY(window->findChild<chrono_calc::ListEditBox*>(QStringLiteral("edit.box")) == nullptr);
        QCOMPARE(list->values().first(), QStringLiteral("6 * 7"));
        // Programmer: greyed, ticked, no list
        QTest::keyClick(area, Qt::Key_3, Qt::AltModifier);
        const QAction* history = actionOf(menuOf(*window, QStringLiteral("&View")), QStringLiteral("Histor&y"));
        QVERIFY(!history->isEnabled());
        QVERIFY(history->isChecked());
        QCOMPARE(area->size(), QSize(508, 413));
        QTest::keyClick(area, Qt::Key_1, Qt::AltModifier);
        QCOMPARE(area->size(), QSize(264, 456));
        window->close();
        window = openWindow();
        QVERIFY(window);
        QCOMPARE(window->centralWidget()->size(), QSize(264, 456));
        QTest::keyClick(window->centralWidget(), Qt::Key_H, Qt::ControlModifier);
        QCOMPARE(window->centralWidget()->size(), QSize(264, 331));
    }

    // The grouping pattern of the regional settings: every three digits in most places, three and then two in India
    static void theGroupingPatternComesFromTheRegionalSettings()
    {
        const QLocale saved;
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
        QCOMPARE(QString::fromStdWString(chrono_calc::MainWindow::systemSeparators().grouping), QStringLiteral("3;0"));
        QLocale::setDefault(QLocale(QLocale::Hindi, QLocale::India));
        const QString india = QString::fromStdWString(chrono_calc::MainWindow::systemSeparators().grouping);
        QLocale::setDefault(saved);
        QCOMPARE(india, QStringLiteral("3;2;0"));
    }

    // While an entry is edited, a click on a calculator button types its character into the edit box (windows7-history.md
    // part D line 4); the ▲ ▼ buttons choose the entry above or below and leave the keyboard focus in the window (part C)
    static void buttonsTypeIntoTheEditBox()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QWidget* area = window->centralWidget();
        QTest::keyClick(area, Qt::Key_H, Qt::ControlModifier);
        QTest::keyClicks(area, QStringLiteral("2+3=2*4="));
        QTRY_VERIFY(window->findChild<chrono_calc::SelectionArrow*>(QStringLiteral("history.up")) != nullptr);
        QTest::mouseClick(window->findChild<chrono_calc::SelectionArrow*>(QStringLiteral("history.up")), Qt::LeftButton);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("5"));
        QVERIFY(!window->findChild<chrono_calc::StatisticsList*>(QStringLiteral("history.list"))->hasFocus());
        QTest::keyClick(area, Qt::Key_F2);
        auto* box = window->findChild<chrono_calc::ListEditBox*>(QStringLiteral("edit.box"));
        QVERIFY(box != nullptr);
        box->setCursorPosition(static_cast<int>(box->text().size()));
        // Keys the box has no use for stay in the box: the edit and the display are kept (found in review)
        QTest::keyClick(box, Qt::Key_F9);
        QTest::keyClick(box, Qt::Key_Up);
        QVERIFY(box->isVisible());
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("5"));
        QTest::mouseClick(buttonOf(*window, QStringLiteral("multiply")), Qt::LeftButton);
        QTest::mouseClick(buttonOf(*window, QStringLiteral("digit.8")), Qt::LeftButton);
        QCOMPARE(box->text(), QStringLiteral("2 + 3*8"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("5"));
        // A button without a character does nothing; = calculates the edit
        QTest::mouseClick(buttonOf(*window, QStringLiteral("sqrt")), Qt::LeftButton);
        QCOMPARE(box->text(), QStringLiteral("2 + 3*8"));
        QTest::mouseClick(buttonOf(*window, QStringLiteral("equals")), Qt::LeftButton);
        QTRY_VERIFY(window->findChild<chrono_calc::ListEditBox*>(QStringLiteral("edit.box")) == nullptr);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("40"));
    }

    // View > Digit grouping groups the number shown, at once and while typing; Copy and Paste use the clipboard
    // (windows7-history.md parts E and F); Digit grouping is remembered
    static void digitGroupingCopyAndPaste()
    {
        auto window = openWindow();
        QVERIFY(window);
        QWidget* area = window->centralWidget();
        QTest::keyClicks(area, QStringLiteral("1234567"));
        QAction* grouping = actionOf(menuOf(*window, QStringLiteral("&View")), QStringLiteral("D&igit grouping"));
        grouping->trigger();
        QVERIFY(grouping->isChecked());
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("1.234.567"));
        QTest::keyClick(area, Qt::Key_C, Qt::ControlModifier);
        QCOMPARE(QGuiApplication::clipboard()->text(), QStringLiteral("1.234.567"));
        QTest::keyClick(area, Qt::Key_Escape);
        QGuiApplication::clipboard()->setText(QStringLiteral("2+3="));
        QTest::keyClick(area, Qt::Key_V, Qt::ControlModifier);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("5"));
        QGuiApplication::clipboard()->setText(QStringLiteral("12345"));
        QTest::keyClick(area, Qt::Key_Insert, Qt::ShiftModifier);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("12.345"));
        QGuiApplication::clipboard()->clear();
        QTest::keyClick(area, Qt::Key_Insert, Qt::ControlModifier);
        QCOMPARE(QGuiApplication::clipboard()->text(), QStringLiteral("12.345"));
        // Paste is greyed while the clipboard holds no text
        QGuiApplication::clipboard()->clear();
        QTRY_VERIFY(!actionOf(menuOf(*window, QStringLiteral("&Edit")), QStringLiteral("&Paste"))->isEnabled());
        QGuiApplication::clipboard()->setText(QStringLiteral("7"));
        QTRY_VERIFY(actionOf(menuOf(*window, QStringLiteral("&Edit")), QStringLiteral("&Paste"))->isEnabled());
        window->close();
        window = openWindow();
        QVERIFY(window);
        QVERIFY(actionOf(menuOf(*window, QStringLiteral("&View")), QStringLiteral("D&igit grouping"))->isChecked());
        QTest::keyClicks(window->centralWidget(), QStringLiteral("1234"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("1.234"));
    }

    // Edit > History: Copy history, Edit and Clear are greyed while the history is empty, Cancel edit unless an entry is
    // edited; F2 and Ctrl+Shift+D do nothing then
    static void historyItemsAreGreyedWhileUnusable()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QWidget* area = window->centralWidget();
        QTest::keyClick(area, Qt::Key_H, Qt::ControlModifier);
        const QMenu* history = actionOf(menuOf(*window, QStringLiteral("&Edit")), QStringLiteral("&History"))->menu();
        const auto enabled = [history](const QString& text) {
            const QAction* action = actionOf(history, text);
            return action != nullptr && action->isEnabled();
        };
        QVERIFY(!enabled(QStringLiteral("Copy h&istory")));
        QVERIFY(!enabled(QStringLiteral("&Edit")));
        QVERIFY(!enabled(QStringLiteral("C&lear")));
        QVERIFY(!enabled(QStringLiteral("Ca&ncel edit")));
        QTest::keyClick(area, Qt::Key_F2);
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("edit.box")) == nullptr);
        QTest::keyClicks(area, QStringLiteral("2+3="));
        QVERIFY(enabled(QStringLiteral("Copy h&istory")));
        QVERIFY(enabled(QStringLiteral("C&lear")));
        QVERIFY(!enabled(QStringLiteral("Ca&ncel edit")));
        // Edit needs a chosen entry
        QTest::keyClick(area, Qt::Key_Up);
        QVERIFY(enabled(QStringLiteral("&Edit")));
        // An entry edited: Cancel edit can be chosen, and drops the edit
        QTest::keyClick(area, Qt::Key_F2);
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("edit.box")) != nullptr);
        QVERIFY(enabled(QStringLiteral("&Recalculate")));
        QVERIFY(enabled(QStringLiteral("Ca&ncel edit")));
        actionOf(history, QStringLiteral("Ca&ncel edit"))->trigger();
        QTRY_VERIFY(partOf<QWidget>(*window, QStringLiteral("edit.box")) == nullptr);
        QVERIFY(!enabled(QStringLiteral("Ca&ncel edit")));
        // Clear empties the history and greys the items again; its key then does nothing
        QTest::keyClick(area, Qt::Key_D, Qt::ControlModifier | Qt::ShiftModifier);
        QVERIFY(window->findChild<chrono_calc::StatisticsList*>(QStringLiteral("history.list"))->values().isEmpty());
        QVERIFY(!enabled(QStringLiteral("Copy h&istory")));
        QVERIFY(!enabled(QStringLiteral("&Edit")));
        QVERIFY(!enabled(QStringLiteral("C&lear")));
        QTest::keyClick(area, Qt::Key_D, Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("5"));
        // Scientific has its own history, empty here
        QTest::keyClicks(area, QStringLiteral("4*5="));
        QVERIFY(enabled(QStringLiteral("C&lear")));
        QTest::keyClick(area, Qt::Key_2, Qt::AltModifier);
        const QMenu* scientific = actionOf(menuOf(*window, QStringLiteral("&Edit")), QStringLiteral("&History"))->menu();
        QVERIFY(!actionOf(scientific, QStringLiteral("C&lear"))->isEnabled());
        QTest::keyClick(area, Qt::Key_H, Qt::ControlModifier);
    }

    // Help > View Help opens the project's page in the browser; About Chrono Calc (F1, D-028) shows the About box with the
    // version, which OK, Escape and Enter close
    static void helpOpensThePageAndTheAboutBox()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QWidget* area = window->centralWidget();
        UrlCatcher catcher;
        QDesktopServices::setUrlHandler(QStringLiteral("https"), &catcher, "open");
        actionOf(menuOf(*window, QStringLiteral("&Help")), QStringLiteral("&View Help"))->trigger();
        QDesktopServices::unsetUrlHandler(QStringLiteral("https"));
        QCOMPARE(catcher.opened(), QStringList({QStringLiteral("https://github.com/rob-vandenberg/chrono-calc")}));

        QTest::keyClicks(area, QStringLiteral("12+"));
        QTest::keyClick(area, Qt::Key_F1);
        auto* box = partOf<QDialog>(*window, QStringLiteral("about.box"));
        QVERIFY(box != nullptr && box->isVisible());
        QCOMPARE(box->windowTitle(), QStringLiteral("About Chrono Calc"));
        QCOMPARE(box->minimumSize(), box->maximumSize());
        QCOMPARE(partOf<QLabel>(*window, QStringLiteral("about.title"))->text(), QStringLiteral("Chrono Calc"));
        QCOMPARE(partOf<QLabel>(*window, QStringLiteral("about.version"))->text(),
                 QStringLiteral("Version ") + QString::fromUtf8(chrono_calc::versionText().data()));
        QVERIFY(partOf<QLabel>(*window, QStringLiteral("about.licence"))->text().contains(QStringLiteral("https://www.gnu.org/licenses/agpl-3.0.html")));
        QVERIFY(partOf<QLabel>(*window, QStringLiteral("about.repository"))->text().contains(QStringLiteral("https://github.com/rob-vandenberg/chrono-calc")));
        QTest::mouseClick(partOf<QWidget>(*window, QStringLiteral("about.ok")), Qt::LeftButton);
        QTRY_VERIFY(partOf<QDialog>(*window, QStringLiteral("about.box")) == nullptr);
        for (const Qt::Key key : {Qt::Key_Escape, Qt::Key_Return}) {
            QTest::keyClick(area, Qt::Key_F1);
            box = partOf<QDialog>(*window, QStringLiteral("about.box"));
            QVERIFY(box != nullptr);
            QTest::keyClick(box, key);
            QTRY_VERIFY(partOf<QDialog>(*window, QStringLiteral("about.box")) == nullptr);
        }
        // The calculation is untouched
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("12"));
        QCOMPARE(displayOf(*window)->expression(), QStringLiteral("12 +"));
    }

    // Each view has its Edit menu (windows7-history.md parts C and G): Programmer's History submenu is greyed (Windows 7
    // leaves it out); Statistics has Copy dataset, Edit, Cancel edit and Clear, and F2 edits the selected value
    static void eachViewHasItsEditMenu()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QWidget* area = window->centralWidget();
        QTest::keyClick(area, Qt::Key_3, Qt::AltModifier);
        const QAction* history = actionOf(menuOf(*window, QStringLiteral("&Edit")), QStringLiteral("&History"));
        QVERIFY(history != nullptr && !history->menu()->isEnabled());
        QTest::keyClick(area, Qt::Key_4, Qt::AltModifier);
        const QMenu* edit = menuOf(*window, QStringLiteral("&Edit"));
        QVERIFY(actionOf(edit, QStringLiteral("&History")) == nullptr);
        for (const QString& item : {QStringLiteral("Copy &dataset"), QStringLiteral("&Edit"), QStringLiteral("Ca&ncel edit"),
                                    QStringLiteral("C&lear")}) {
            QVERIFY2(actionOf(edit, item) != nullptr && actionOf(edit, item)->isEnabled(), qPrintable(item));
        }
        for (const QString& value : {QStringLiteral("5"), QStringLiteral("3"), QStringLiteral("4")}) {
            QTest::keyClicks(area, value);
            QTest::keyClick(area, Qt::Key_Return);
        }
        QTest::keyClick(area, Qt::Key_F2);
        auto* box = window->findChild<chrono_calc::ListEditBox*>(QStringLiteral("edit.box"));
        QVERIFY(box != nullptr);
        QCOMPARE(box->text(), QStringLiteral("4"));
        QVERIFY(!actionOf(edit, QStringLiteral("&Edit"))->isEnabled());
        box->selectAll();
        QTest::keyClicks(box, QStringLiteral("7"));
        QTest::keyClick(box, Qt::Key_Return);
        auto* list = window->findChild<chrono_calc::StatisticsList*>(QStringLiteral("statistics.list"));
        QCOMPARE(list->values(), QStringList({QStringLiteral("5"), QStringLiteral("3"), QStringLiteral("7")}));
        QTRY_VERIFY(list->hasFocus());
        actionOf(edit, QStringLiteral("Copy &dataset"))->trigger();
        QCOMPARE(QGuiApplication::clipboard()->text(), QStringLiteral("5\n3\n7\n"));
    }

    // The window opens in the view it was closed in, as Windows 7 does
    static void theWindowOpensInTheViewItWasClosedIn()
    {
        {
            const auto window = openWindow();
            QVERIFY(window);
            QTest::keyClick(window->centralWidget(), Qt::Key_2, Qt::AltModifier);
            window->close();
        }
        const auto window = openWindow();
        QVERIFY(window);
        QCOMPARE(window->centralWidget()->size(), QSize(508, 331));
        QVERIFY(actionOf(menuOf(*window, QStringLiteral("&View")), QStringLiteral("&Scientific"))->isChecked());
    }

    // View > Unit conversion (Ctrl+U): the panel beside every view, the window wider by it (windows7-unitconversion.md
    // part A lines 1 and 4); Ctrl+U again keeps it; Basic (Ctrl+F4) hides it
    static void theUnitConversionPanelShowsBesideEveryView()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QWidget* area = window->centralWidget();
        QTest::keyClick(area, Qt::Key_U, Qt::ControlModifier);
        const QWidget* panel = partOf<QWidget>(*window, QStringLiteral("unit.panel"));
        QVERIFY(panel != nullptr);
        QCOMPARE(area->size(), QSize(731, 331));
        QCOMPARE(panel->geometry(), QRect(264, 13, 454, 303));
        QVERIFY(actionOf(menuOf(*window, QStringLiteral("&View")), QStringLiteral("&Unit conversion"))->isChecked());
        QTest::keyClick(area, Qt::Key_U, Qt::ControlModifier);
        QCOMPARE(area->size(), QSize(731, 331));
        struct Place
        {
            Qt::Key key = Qt::Key_unknown;
            QRect panel;
            QSize client;
        };
        for (const Place& place : {Place{.key = Qt::Key_2, .panel = QRect(508, 13, 454, 303), .client = QSize(975, 331)},
                                   Place{.key = Qt::Key_3, .panel = QRect(508, 13, 454, 386), .client = QSize(975, 413)},
                                   Place{.key = Qt::Key_4, .panel = QRect(264, 13, 454, 427), .client = QSize(731, 456)}}) {
            QTest::keyClick(area, place.key, Qt::AltModifier);
            const QWidget* shown = partOf<QWidget>(*window, QStringLiteral("unit.panel"));
            QVERIFY(shown != nullptr);
            QCOMPARE(shown->geometry(), place.panel);
            QCOMPARE(area->size(), place.client);
        }
        QTest::keyClick(area, Qt::Key_1, Qt::AltModifier);
        QTest::keyClick(area, Qt::Key_H, Qt::ControlModifier);
        QCOMPARE(area->size(), QSize(731, 456));
        QCOMPARE(partOf<QWidget>(*window, QStringLiteral("unit.panel"))->geometry(), QRect(264, 13, 454, 428));
        QTest::keyClick(area, Qt::Key_H, Qt::ControlModifier);
        QTest::keyClick(area, Qt::Key_F4, Qt::ControlModifier);
        QCOMPARE(area->size(), QSize(264, 331));
        QVERIFY(actionOf(menuOf(*window, QStringLiteral("&View")), QStringLiteral("&Basic"))->isChecked());
    }

    // The From box converts at every character; refused characters never show; the display stays as it is (part D
    // lines 1, 2 and 5)
    static void theUnitConversionPanelConverts()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClick(window->centralWidget(), Qt::Key_U, Qt::ControlModifier);
        auto* type = partOf<QComboBox>(*window, QStringLiteral("unit.type"));
        auto* from = partOf<QComboBox>(*window, QStringLiteral("unit.from"));
        auto* to = partOf<QComboBox>(*window, QStringLiteral("unit.to"));
        auto* input = partOf<QLineEdit>(*window, QStringLiteral("unit.input"));
        const auto* result = partOf<QLineEdit>(*window, QStringLiteral("unit.result"));
        QVERIFY(type != nullptr && from != nullptr && to != nullptr && input != nullptr && result != nullptr);
        QCOMPARE(type->currentText(), QStringLiteral("Angle"));
        QCOMPARE(from->currentText(), QStringLiteral("Degree"));
        QCOMPARE(to->currentText(), QStringLiteral("Degree"));
        type->setCurrentIndex(3);
        QCOMPARE(from->currentText(), QStringLiteral("Angstrom"));
        QCOMPARE(from->count(), 19);
        from->setCurrentIndex(from->findText(QStringLiteral("Inch")));
        to->setCurrentIndex(to->findText(QStringLiteral("Centimeters")));
        input->setFocus();
        QTest::keyClicks(input, QStringLiteral("1"));
        QCOMPARE(result->text(), QStringLiteral("2,54"));
        QTest::keyClicks(input, QStringLiteral("2a"));
        QCOMPARE(input->text(), QStringLiteral("12"));
        QCOMPARE(result->text(), QStringLiteral("30,48"));
        QTest::keyClicks(input, QStringLiteral("3"));
        QCOMPARE(result->text(), QStringLiteral("312,42"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));
        // F9, Enter and Escape in the From box do nothing, and nothing reaches the calculator
        for (const Qt::Key key : {Qt::Key_F9, Qt::Key_Return, Qt::Key_Escape}) {
            QTest::keyClick(input, key);
        }
        QCOMPARE(input->text(), QStringLiteral("123"));
        QVERIFY(input->hasFocus());
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));
        // A type chosen empties both boxes
        type->setCurrentIndex(0);
        QVERIFY(input->text().isEmpty());
        QVERIFY(result->text().isEmpty());
    }

    // With a list focused the keys go to the calculator; Up and Down change the unit; Enter and Escape give the focus
    // back to the calculator (part D line 7); the Tab order is Windows 7's
    static void theUnitListsPassTheirKeysToTheCalculator()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QWidget* area = window->centralWidget();
        QTest::keyClick(area, Qt::Key_U, Qt::ControlModifier);
        auto* from = partOf<QComboBox>(*window, QStringLiteral("unit.from"));
        QVERIFY(from != nullptr);
        from->setFocus();
        QTest::keyClicks(from, QStringLiteral("78"));
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("78"));
        QTest::keyClick(from, Qt::Key_Down);
        QCOMPARE(from->currentText(), QStringLiteral("Gradian"));
        QTest::keyClick(from, Qt::Key_Escape);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));
        QVERIFY(area->hasFocus());
        // Tab from each part reaches the next: From, its unit, To, its unit, the calculator, the type list, From again
        const std::vector<QWidget*> order{partOf<QWidget>(*window, QStringLiteral("unit.input")), from,
                                          partOf<QWidget>(*window, QStringLiteral("unit.result")),
                                          partOf<QWidget>(*window, QStringLiteral("unit.to")), area,
                                          partOf<QWidget>(*window, QStringLiteral("unit.type")),
                                          partOf<QWidget>(*window, QStringLiteral("unit.input"))};
        for (std::size_t index = 0; index + 1 < order.size(); ++index) {
            order.at(index)->setFocus();
            QTest::keyClick(order.at(index), Qt::Key_Tab);
            QVERIFY2(order.at(index + 1)->hasFocus(), qPrintable(QString::number(index)));
        }
    }

    // The panel shown is remembered, not its type (part D line 10)
    static void theWindowOpensWithTheSidePanelItWasClosedWith()
    {
        {
            const auto window = openWindow();
            QVERIFY(window);
            QTest::keyClick(window->centralWidget(), Qt::Key_U, Qt::ControlModifier);
            partOf<QComboBox>(*window, QStringLiteral("unit.type"))->setCurrentIndex(3);
            window->close();
        }
        const auto window = openWindow();
        QVERIFY(window);
        QCOMPARE(window->centralWidget()->size(), QSize(731, 331));
        QCOMPARE(partOf<QComboBox>(*window, QStringLiteral("unit.type"))->currentText(), QStringLiteral("Angle"));
    }

    // View > Date calculation (Ctrl+E): the panel in the frame of Unit conversion, beside every view; Ctrl+E again keeps
    // it; the mode list has the keyboard focus (windows7-datecalculation.md part A lines 1 and 4)
    static void theDateCalculationPanelShowsBesideEveryView()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QWidget* area = window->centralWidget();
        QTest::keyClick(area, Qt::Key_E, Qt::ControlModifier);
        const QWidget* panel = partOf<QWidget>(*window, QStringLiteral("date.panel"));
        QVERIFY(panel != nullptr);
        QCOMPARE(area->size(), QSize(731, 331));
        QCOMPARE(panel->geometry(), QRect(264, 13, 454, 303));
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("date.mode"))->hasFocus());
        QVERIFY(actionOf(menuOf(*window, QStringLiteral("&View")), QStringLiteral("&Date calculation"))->isChecked());
        QTest::keyClick(area, Qt::Key_E, Qt::ControlModifier);
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("date.panel")) != nullptr);
        QTest::keyClick(area, Qt::Key_3, Qt::AltModifier);
        QCOMPARE(partOf<QWidget>(*window, QStringLiteral("date.panel"))->geometry(), QRect(508, 13, 454, 386));
        QCOMPARE(area->size(), QSize(975, 413));
        // From Unit conversion to Date calculation and back: one panel at a time
        QTest::keyClick(area, Qt::Key_U, Qt::ControlModifier);
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("unit.panel")) != nullptr);
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("date.panel")) == nullptr);
        QTest::keyClick(area, Qt::Key_E, Qt::ControlModifier);
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("unit.panel")) == nullptr);
        QTest::keyClick(area, Qt::Key_F4, Qt::ControlModifier);
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("date.panel")) == nullptr);
        QCOMPARE(area->size(), QSize(508, 413));
    }

    // Both modes: their parts, Calculate, the results in the regional long date; a changed date empties the results
    // (parts B, D and E)
    static void theDateCalculationPanelCalculates()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClick(window->centralWidget(), Qt::Key_E, Qt::ControlModifier);
        auto* mode = partOf<QComboBox>(*window, QStringLiteral("date.mode"));
        auto* from = partOf<QDateEdit>(*window, QStringLiteral("date.difference_from"));
        auto* to = partOf<QDateEdit>(*window, QStringLiteral("date.difference_to"));
        const auto* difference = partOf<QLineEdit>(*window, QStringLiteral("date.difference"));
        const auto* days = partOf<QLineEdit>(*window, QStringLiteral("date.total_days"));
        auto* calculate = partOf<QWidget>(*window, QStringLiteral("date.calculate"));
        QVERIFY(mode != nullptr && from != nullptr && to != nullptr && difference != nullptr && days != nullptr && calculate != nullptr);
        QCOMPARE(mode->count(), 2);
        QCOMPARE(mode->itemText(0), QStringLiteral("Calculate the difference between two dates"));
        QCOMPARE(mode->itemText(1), QStringLiteral("Add or subtract days to a specified date"));
        // Today, or yesterday when midnight passed since the window opened
        QVERIFY(from->date() == QDate::currentDate() || from->date() == QDate::currentDate().addDays(-1));
        QCOMPARE(from->minimumDate(), QDate(1601, 1, 1));
        QCOMPARE(from->maximumDate(), QDate(9999, 12, 31));
        QVERIFY(from->displayFormat().contains(QStringLiteral("yyyy")));
        from->setDate(QDate(2026, 1, 15));
        to->setDate(QDate(2027, 3, 20));
        QVERIFY(difference->text().isEmpty());
        QTest::mouseClick(calculate, Qt::LeftButton);
        QCOMPARE(difference->text(), QStringLiteral("1 year, 2 months, 5 days"));
        QCOMPARE(days->text(), QStringLiteral("429 days"));
        to->setDate(QDate(2027, 3, 21));
        QVERIFY(difference->text().isEmpty());
        QVERIFY(days->text().isEmpty());
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));

        mode->setCurrentIndex(1);
        QVERIFY(!from->isVisible());
        auto* shiftFrom = partOf<QDateEdit>(*window, QStringLiteral("date.shift_from"));
        auto* subtract = partOf<QRadioButton>(*window, QStringLiteral("date.subtract"));
        auto* years = partOf<QLineEdit>(*window, QStringLiteral("date.years"));
        const auto* shifted = partOf<QLineEdit>(*window, QStringLiteral("date.shifted"));
        QVERIFY(shiftFrom != nullptr && subtract != nullptr && years != nullptr && shifted != nullptr);
        QVERIFY(shiftFrom->isVisible());
        QCOMPARE(years->text(), QStringLiteral("0"));
        shiftFrom->setDate(QDate(2026, 3, 31));
        subtract->click();
        years->setFocus();
        years->selectAll();
        QTest::keyClicks(years, QStringLiteral("1"));
        QTest::mouseClick(calculate, Qt::LeftButton);
        QCOMPARE(shifted->text(), QLocale().toString(QDate(2025, 3, 31), QLocale::LongFormat));
        // The other mode kept its result
        mode->setCurrentIndex(0);
        QVERIFY(from->isVisible());
        QCOMPARE(from->date(), QDate(2026, 1, 15));
        QVERIFY(difference->text().isEmpty());
    }

    // A number box keeps its keys: digits at most three, Up and Down step it, Enter calculates and moves the keyboard
    // focus to the result; nothing reaches the calculator (part E line 1, part F line 2)
    static void theNumberBoxesKeepTheirKeys()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClick(window->centralWidget(), Qt::Key_E, Qt::ControlModifier);
        partOf<QComboBox>(*window, QStringLiteral("date.mode"))->setCurrentIndex(1);
        partOf<QDateEdit>(*window, QStringLiteral("date.shift_from"))->setDate(QDate(2026, 10, 30));
        auto* days = partOf<QLineEdit>(*window, QStringLiteral("date.days"));
        const auto* shifted = partOf<QLineEdit>(*window, QStringLiteral("date.shifted"));
        QVERIFY(days != nullptr && shifted != nullptr);
        days->setFocus();
        days->selectAll();
        QTest::keyClicks(days, QStringLiteral("+5a*99"));
        QCOMPARE(days->text(), QStringLiteral("599"));
        QTest::keyClick(days, Qt::Key_Up);
        QCOMPARE(days->text(), QStringLiteral("600"));
        QTest::keyClick(days, Qt::Key_Down);
        QTest::keyClick(days, Qt::Key_Down);
        QCOMPARE(days->text(), QStringLiteral("598"));
        QTest::keyClick(days, Qt::Key_Escape);
        QVERIFY(days->hasFocus());
        QTest::keyClick(days, Qt::Key_Return);
        QCOMPARE(shifted->text(), QLocale().toString(QDate(2028, 6, 19), QLocale::LongFormat));
        QVERIFY(shifted->hasFocus());
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));
        QCOMPARE(displayOf(*window)->expression(), QString());
    }

    // The arrows beside a number box: the upper half steps up, the lower half down, and they never take the focus
    static void theNumberArrowsStep()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClick(window->centralWidget(), Qt::Key_E, Qt::ControlModifier);
        partOf<QComboBox>(*window, QStringLiteral("date.mode"))->setCurrentIndex(1);
        const auto* years = partOf<QLineEdit>(*window, QStringLiteral("date.years"));
        QVERIFY(years != nullptr);
        // The arrows of Year(s) at 120, 120, 18, 28 in the panel, 45 below the part's top (75)
        QWidget* arrows = partOf<QWidget>(*window, QStringLiteral("date.panel"))->childAt(QPoint(128, 125));
        QVERIFY(arrows != nullptr);
        QTest::mouseClick(arrows, Qt::LeftButton, Qt::NoModifier, QPoint(9, 4));
        QCOMPARE(years->text(), QStringLiteral("1"));
        QTest::mouseClick(arrows, Qt::LeftButton, Qt::NoModifier, QPoint(9, 24));
        QTest::mouseClick(arrows, Qt::LeftButton, Qt::NoModifier, QPoint(9, 24));
        QCOMPARE(years->text(), QStringLiteral("999"));
        QVERIFY(!arrows->hasFocus());
    }

    // Enter in a date box takes a digit that still waits for its second one before it calculates (found in review); a
    // two-digit day, as in the reference's regional settings, so that the 3 waits whatever the test computer's locale
    static void enterTakesTheWaitingDigit()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClick(window->centralWidget(), Qt::Key_E, Qt::ControlModifier);
        auto* from = partOf<QDateEdit>(*window, QStringLiteral("date.difference_from"));
        auto* to = partOf<QDateEdit>(*window, QStringLiteral("date.difference_to"));
        QVERIFY(from != nullptr && to != nullptr);
        from->setDisplayFormat(QStringLiteral("dd/MM/yyyy"));
        from->setDate(QDate(2026, 4, 1));
        to->setDate(QDate(2026, 4, 20));
        from->setFocus();
        from->setSelectedSection(QDateTimeEdit::DaySection);
        QTest::keyClick(from, Qt::Key_3);
        QTest::keyClick(from, Qt::Key_Return);
        QCOMPARE(from->date(), QDate(2026, 4, 3));
        QCOMPARE(partOf<QLineEdit>(*window, QStringLiteral("date.total_days"))->text(), QStringLiteral("17 days"));
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("date.difference"))->hasFocus());
    }

    // Windows 7's Tab order in both modes (part F line 1); Enter in a date box calculates and moves the keyboard focus
    // to the first result
    static void theDateCalculationTabOrderIsWindows7s()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QWidget* area = window->centralWidget();
        QTest::keyClick(area, Qt::Key_E, Qt::ControlModifier);
        const auto part = [&window](const char* name) { return partOf<QWidget>(*window, QString::fromUtf8(name)); };
        const std::vector<QWidget*> differenceOrder{part("date.difference_from"), part("date.difference_to"), part("date.difference"),
                                                    part("date.total_days"), part("date.calculate"), area, part("date.mode")};
        for (std::size_t index = 0; index + 1 < differenceOrder.size(); ++index) {
            differenceOrder.at(index)->setFocus();
            QTest::keyClick(differenceOrder.at(index), Qt::Key_Tab);
            QVERIFY2(differenceOrder.at(index + 1)->hasFocus(), qPrintable(QString::number(index)));
        }
        QTest::keyClick(part("date.mode"), Qt::Key_Tab);
        QVERIFY(part("date.difference_from")->hasFocus());
        QTest::keyClick(part("date.difference_from"), Qt::Key_Return);
        QVERIFY(part("date.difference")->hasFocus());
        QCOMPARE(qobject_cast<QLineEdit*>(part("date.difference"))->text(), QStringLiteral("Same dates"));
        // Right on Add chooses Subtract
        qobject_cast<QComboBox*>(part("date.mode"))->setCurrentIndex(1);
        part("date.add")->setFocus();
        QTest::keyClick(part("date.add"), Qt::Key_Right);
        QVERIFY(qobject_cast<QRadioButton*>(part("date.subtract"))->isChecked());
        QVERIFY(part("date.subtract")->hasFocus());

        const std::vector<QWidget*> shiftOrder{part("date.shift_from"), part("date.subtract"), part("date.years"), part("date.months"),
                                               part("date.days"), part("date.shifted"), part("date.calculate"), area};
        for (std::size_t index = 0; index + 1 < shiftOrder.size(); ++index) {
            shiftOrder.at(index)->setFocus();
            QTest::keyClick(shiftOrder.at(index), Qt::Key_Tab);
            QVERIFY2(shiftOrder.at(index + 1)->hasFocus(), qPrintable(QString::number(index)));
        }
    }

    // The panel shown is remembered, not its dates or mode (part G line 2)
    static void theWindowOpensWithTheDateCalculationPanel()
    {
        {
            const auto window = openWindow();
            QVERIFY(window);
            QTest::keyClick(window->centralWidget(), Qt::Key_E, Qt::ControlModifier);
            partOf<QComboBox>(*window, QStringLiteral("date.mode"))->setCurrentIndex(1);
            window->close();
        }
        const auto window = openWindow();
        QVERIFY(window);
        QCOMPARE(window->centralWidget()->size(), QSize(731, 331));
        QCOMPARE(partOf<QComboBox>(*window, QStringLiteral("date.mode"))->currentIndex(), 0);
    }

    // windows7-worksheets.md part A: the menu items without a dot, the panel beside every view, the boxes per worksheet
    static void theWorksheetsShowBesideEveryView()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QWidget* area = window->centralWidget();
        QAction* mortgage = worksheetAction(*window, QStringLiteral("&Mortgage"));
        QVERIFY(mortgage != nullptr && mortgage->isEnabled() && !mortgage->isCheckable());
        for (const char* text : {"&Vehicle lease", "&Fuel economy (mpg)", "F&uel economy (L/100 km)"}) {
            QVERIFY(worksheetAction(*window, QString::fromUtf8(text)) != nullptr);
        }
        mortgage->trigger();
        const QWidget* panel = partOf<QWidget>(*window, QStringLiteral("worksheet.panel"));
        QVERIFY(panel != nullptr);
        QCOMPARE(area->size(), QSize(731, 331));
        QCOMPARE(panel->geometry(), QRect(264, 13, 454, 303));
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("worksheet.list"))->hasFocus());
        const QMenu* view = menuOf(*window, QStringLiteral("&View"));
        for (const char* text : {"&Basic", "&Unit conversion", "&Date calculation"}) {
            QVERIFY(!actionOf(view, QString::fromUtf8(text))->isChecked());
        }
        QCOMPARE(partOf<QWidget>(*window, QStringLiteral("worksheet.box.0"))->geometry(), QRect(225, 75, 217, 27));
        QCOMPARE(partOf<QWidget>(*window, QStringLiteral("worksheet.box.3"))->geometry(), QRect(225, 169, 217, 27));
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("worksheet.box.4")) == nullptr);
        QCOMPARE(partOf<QWidget>(*window, QStringLiteral("worksheet.calculate"))->geometry(), QRect(12, 263, 97, 28));
        QCOMPARE(partOf<QWidget>(*window, QStringLiteral("worksheet.result"))->geometry(), QRect(225, 265, 217, 27));
        QTest::keyClick(area, Qt::Key_3, Qt::AltModifier);
        QCOMPARE(partOf<QWidget>(*window, QStringLiteral("worksheet.panel"))->geometry(), QRect(508, 13, 454, 386));
        QCOMPARE(area->size(), QSize(975, 413));
        worksheetAction(*window, QStringLiteral("&Vehicle lease"))->trigger();
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("worksheet.box.4")) != nullptr);
        worksheetAction(*window, QStringLiteral("&Fuel economy (mpg)"))->trigger();
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("worksheet.box.1")) != nullptr);
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("worksheet.box.2")) == nullptr);
        QTest::keyClick(area, Qt::Key_F4, Qt::ControlModifier);
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("worksheet.panel")) == nullptr);
        QVERIFY(actionOf(view, QStringLiteral("&Basic"))->isChecked());
        QCOMPARE(area->size(), QSize(508, 413));
    }

    // Part B lines 1 to 4: the labels follow the value chosen, Calculate and Enter, the focus, the messages
    static void theWorksheetCalculates()
    {
        const auto window = openWindow();
        QVERIFY(window);
        worksheetAction(*window, QStringLiteral("&Mortgage"))->trigger();
        auto* list = partOf<QComboBox>(*window, QStringLiteral("worksheet.list"));
        auto* result = partOf<QLineEdit>(*window, QStringLiteral("worksheet.result"));
        auto* calculate = partOf<QWidget>(*window, QStringLiteral("worksheet.calculate"));
        QVERIFY(list != nullptr && result != nullptr && calculate != nullptr);
        QCOMPARE(list->count(), 4);
        QCOMPARE(list->currentText(), QStringLiteral("Monthly payment"));
        const auto box = [&window](int index) { return partOf<QLineEdit>(*window, QStringLiteral("worksheet.box.%1").arg(index)); };
        QCOMPARE(box(0)->placeholderText(), QStringLiteral("Enter value"));
        const QStringList typed{QStringLiteral("250000"), QStringLiteral("50000"), QStringLiteral("30"), QStringLiteral("4,5")};
        for (int index = 0; index < typed.size(); ++index) {
            box(index)->setFocus();
            QTest::keyClicks(box(index), typed.at(index));
        }
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));
        QTest::mouseClick(calculate, Qt::LeftButton);
        QCOMPARE(result->text(), QStringLiteral("1013,370619651761"));
        QVERIFY(result->hasFocus());
        // Term (years): the labels move up, the old result becomes the last box
        list->setCurrentIndex(3);
        const auto label = [&window](int index) {
            return partOf<QLabel>(*window, QStringLiteral("worksheet.label.%1").arg(index))->text();
        };
        QCOMPARE(label(2), QStringLiteral("Interest rate (%)"));
        QCOMPARE(label(3), QStringLiteral("Monthly payment"));
        QCOMPARE(box(3)->text(), QStringLiteral("1013,370619651761"));
        QCOMPARE(result->text(), QStringLiteral("30"));
        // Enter in a box calculates; an empty box: the message, the focus there, the result emptied
        box(1)->setFocus();
        box(1)->selectAll();
        QTest::keyClick(box(1), Qt::Key_Delete);
        QTest::keyClick(box(1), Qt::Key_Return);
        QVERIFY(box(1)->hasFocus());
        QVERIFY(result->text().isEmpty());
        const auto* balloon = partOf<QWidget>(*window, QStringLiteral("worksheet.balloon"));
        QVERIFY(balloon != nullptr && balloon->isVisible());
        // Typing keeps the balloon; a valid text calculates and the balloon goes
        QTest::keyClicks(box(1), QStringLiteral("50000"));
        QVERIFY(balloon->isVisible());
        QTest::keyClick(box(1), Qt::Key_Return);
        QCOMPARE(result->text(), QStringLiteral("30,00000000000002"));
        QVERIFY(!balloon->isVisible());
        // A text that is not valid keeps the result
        box(2)->setFocus();
        box(2)->selectAll();
        QTest::keyClicks(box(2), QStringLiteral("-5"));
        QTest::keyClick(box(2), Qt::Key_Return);
        QCOMPARE(result->text(), QStringLiteral("30,00000000000002"));
        QVERIFY(box(2)->hasFocus() && balloon->isVisible());
        QTest::keyClick(box(2), Qt::Key_Right);
        QVERIFY(!balloon->isVisible());
        for (const Qt::Key key : {Qt::Key_Left, Qt::Key_Home, Qt::Key_End}) {
            QTest::keyClick(box(2), Qt::Key_Return);
            QVERIFY(balloon->isVisible());
            QTest::keyClick(box(2), key);
            QVERIFY(!balloon->isVisible());
        }
        // The balloon does not follow a moved window: it goes
        QTest::keyClick(box(2), Qt::Key_Return);
        QVERIFY(balloon->isVisible());
        window->move(window->pos() + QPoint(40, 30));
        QTRY_VERIFY(!balloon->isVisible());
        // A 26th typed character is refused; a pasted text is filtered first, then cut to 25 characters
        box(0)->setFocus();
        box(0)->selectAll();
        QTest::keyClicks(box(0), QStringLiteral("1234567890123456789012345"));
        QTest::keyClick(box(0), Qt::Key_Home);
        QTest::keyClick(box(0), Qt::Key_9);
        QCOMPARE(box(0)->text(), QStringLiteral("1234567890123456789012345"));
        box(0)->clear();
        QTest::keyClicks(box(0), QStringLiteral("12345678901234567890"));
        QGuiApplication::clipboard()->setText(QStringLiteral("1 234 5"));
        box(0)->paste();
        QCOMPARE(box(0)->text(), QStringLiteral("1234567890123456789012345"));
        // Another value chosen takes the balloon away; Escape on the list gives the calculator the keyboard focus
        QTest::keyClick(box(2), Qt::Key_Return);
        QVERIFY(balloon->isVisible());
        list->setCurrentIndex(1);
        QVERIFY(!balloon->isVisible());
        list->setFocus();
        QTest::keyClick(list, Qt::Key_Escape);
        QVERIFY(window->centralWidget()->hasFocus());
    }

    // Part F line 2: the list keeps the keys from the calculator; Up and Down change the value, Enter calculates
    static void theWorksheetListKeepsItsKeys()
    {
        const auto window = openWindow();
        QVERIFY(window);
        worksheetAction(*window, QStringLiteral("&Fuel economy (mpg)"))->trigger();
        auto* list = partOf<QComboBox>(*window, QStringLiteral("worksheet.list"));
        QVERIFY(list != nullptr && list->hasFocus());
        QTest::keyClick(list, Qt::Key_7);
        QTest::keyClick(list, Qt::Key_Plus);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("0"));
        QCOMPARE(list->currentText(), QStringLiteral("Fuel economy (mpg)"));
        QTest::keyClick(list, Qt::Key_Down);
        QCOMPARE(list->currentText(), QStringLiteral("Fuel used (gallons)"));
        QTest::keyClick(list, Qt::Key_Return);
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("worksheet.box.0"))->hasFocus());
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("worksheet.balloon"))->isVisible());
    }

    // Part F line 1: the boxes, Calculate, the result, the calculator, the list, the first box
    static void theWorksheetTabOrderIsWindows7s()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QWidget* area = window->centralWidget();
        worksheetAction(*window, QStringLiteral("&Mortgage"))->trigger();
        const auto part = [&window](const QString& name) { return partOf<QWidget>(*window, name); };
        const std::vector<QWidget*> order{part(QStringLiteral("worksheet.box.0")), part(QStringLiteral("worksheet.box.1")),
                                          part(QStringLiteral("worksheet.box.2")), part(QStringLiteral("worksheet.box.3")),
                                          part(QStringLiteral("worksheet.calculate")), part(QStringLiteral("worksheet.result")), area,
                                          part(QStringLiteral("worksheet.list")), part(QStringLiteral("worksheet.box.0"))};
        for (std::size_t index = 0; index + 1 < order.size(); ++index) {
            order.at(index)->setFocus();
            QTest::keyClick(order.at(index), Qt::Key_Tab);
            QVERIFY2(order.at(index + 1)->hasFocus(), qPrintable(QString::number(index)));
        }
    }

    // The worksheet shown is remembered, not its boxes (part B lines 6 and 7)
    static void theWindowOpensWithTheWorksheet()
    {
        {
            const auto window = openWindow();
            QVERIFY(window);
            worksheetAction(*window, QStringLiteral("&Vehicle lease"))->trigger();
            QTest::keyClicks(partOf<QLineEdit>(*window, QStringLiteral("worksheet.box.0")), QStringLiteral("5"));
            window->close();
        }
        const auto window = openWindow();
        QVERIFY(window);
        QCOMPARE(window->centralWidget()->size(), QSize(731, 331));
        QVERIFY(partOf<QWidget>(*window, QStringLiteral("worksheet.box.4")) != nullptr);
        QVERIFY(partOf<QLineEdit>(*window, QStringLiteral("worksheet.box.0"))->text().isEmpty());
        QTest::keyClick(window->centralWidget(), Qt::Key_F4, Qt::ControlModifier);
    }

    // View > Basic changes nothing on the display
    static void basicKeepsTheNumber()
    {
        const auto window = openWindow();
        QVERIFY(window);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("2+3"));
        actionOf(menuOf(*window, QStringLiteral("&View")), QStringLiteral("&Basic"))->trigger();
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("3"));
        QCOMPARE(displayOf(*window)->expression(), QStringLiteral("2 +"));
    }

    // The window opens where it was closed
    static void theWindowOpensWhereItWasClosed()
    {
        {
            const auto window = openWindow();
            QVERIFY(window);
            window->move(QPoint(140, 90));
            QTRY_COMPARE(window->pos(), QPoint(140, 90));
            window->close();
        }
        const auto window = openWindow();
        QVERIFY(window);
        QCOMPARE(window->pos(), QPoint(140, 90));
    }

    // A position on no screen any more (a screen that was removed) is not used
    static void aPositionOnNoScreenIsNotUsed()
    {
        QSettings().setValue(QStringLiteral("window/position"), QPoint(-30000, -30000));
        const auto window = openWindow();
        QVERIFY(window);
        QVERIFY(window->pos() != QPoint(-30000, -30000));
    }

    // A position where only a strip of the window would show (its screen was beside this one) is not used
    static void aPositionMostlyOffTheScreenIsNotUsed()
    {
        const QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
        const QPoint edge(screen.right() - 5, screen.top() + 50);
        QSettings().setValue(QStringLiteral("window/position"), edge);
        const auto window = openWindow();
        QVERIFY(window);
        QVERIFY(window->pos() != edge);
    }

    // A key typed while the mouse holds a button down leaves the button down, so the click still comes
    static void aTypedKeyKeepsAButtonTheMouseHolds()
    {
        const auto window = openWindow();
        QVERIFY(window);
        chrono_calc::CalcButton* seven = buttonOf(*window, QStringLiteral("digit.7"));
        QTest::mousePress(seven, Qt::LeftButton);
        QTest::keyClicks(window->centralWidget(), QStringLiteral("7"));
        QTest::qWait(200);
        QVERIFY(seven->isDown());
        QTest::mouseRelease(seven, Qt::LeftButton);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("77"));
    }

    // The other order: the mouse presses a button while it still flashes from its key; the click still comes
    static void aMouseClickDuringAFlashStillCounts()
    {
        const auto window = openWindow();
        QVERIFY(window);
        chrono_calc::CalcButton* seven = buttonOf(*window, QStringLiteral("digit.7"));
        QTest::keyClicks(window->centralWidget(), QStringLiteral("7"));
        QTest::mousePress(seven, Qt::LeftButton);
        QTest::qWait(200);
        QVERIFY(seven->isDown());
        QTest::mouseRelease(seven, Qt::LeftButton);
        QCOMPARE(displayOf(*window)->number(), QStringLiteral("77"));
    }
};

QTEST_MAIN(TestMainWindow)
#include "test_mainwindow.moc"
