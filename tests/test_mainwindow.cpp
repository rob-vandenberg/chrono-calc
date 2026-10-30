#include <QAction>
#include <QClipboard>
#include <QCoreApplication>
#include <QEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QList>
#include <QLocale>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QWidget>

#include "core/enginetexts.h"
#include "ui/calcbutton.h"
#include "ui/displaypanel.h"
#include "ui/mainwindow.h"
#include "ui/statisticsparts.h"
#include "ui/viewparts.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <memory>

// --- Version ------------------------------------------------------------
// tests/test_mainwindow.cpp 0.5.12

// --- Version history ----------------------------------------------------
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
        for (const QString& greyed : {QStringLiteral("&Unit conversion"), QStringLiteral("&Date calculation")}) {
            const QAction* action = actionOf(view, greyed);
            QVERIFY2(action != nullptr && !action->isEnabled(), qPrintable(greyed));
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
        for (const QAction* action : history->actions()) {
            QVERIFY(action->isEnabled());
        }
        // F2 and Esc are only shown: the window's keys do them (Esc is C, F2 a word size in Programmer)
        QVERIFY(actionOf(history, QStringLiteral("&Edit"))->shortcut().isEmpty());
        QCOMPARE(actionOf(history, QStringLiteral("&Edit"))->text(), QStringLiteral("&Edit\tF2"));
        QCOMPARE(actionOf(history, QStringLiteral("C&lear"))->shortcut(), QKeySequence(QStringLiteral("Ctrl+Shift+D")));

        const QMenu* help = menuOf(*window, QStringLiteral("&Help"));
        QVERIFY(help != nullptr);
        QVERIFY(!actionOf(help, QStringLiteral("&View Help"))->isEnabled());
        QVERIFY(!actionOf(help, QStringLiteral("&About Chrono Calc"))->isEnabled());
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
