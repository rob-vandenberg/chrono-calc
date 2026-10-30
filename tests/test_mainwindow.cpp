#include <QAction>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QKeySequence>
#include <QList>
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

#include "core/enginetexts.h"
#include "ui/calcbutton.h"
#include "ui/displaypanel.h"
#include "ui/mainwindow.h"
#include "ui/viewparts.h"

#include <algorithm>
#include <string>
#include <memory>

// --- Version ------------------------------------------------------------
// tests/test_mainwindow.cpp 0.3.8

// --- Version history ----------------------------------------------------
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

}  // namespace

class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    static void initTestCase()
    {
        settingsFolder() = std::make_unique<QTemporaryDir>();
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
        for (const QString& greyed : {QStringLiteral("St&atistics"),
                                      QStringLiteral("Histor&y"), QStringLiteral("D&igit grouping"), QStringLiteral("&Unit conversion"),
                                      QStringLiteral("&Date calculation")}) {
            const QAction* action = actionOf(view, greyed);
            QVERIFY2(action != nullptr && !action->isEnabled(), qPrintable(greyed));
        }
        QCOMPARE(actionOf(view, QStringLiteral("St&atistics"))->text(), QStringLiteral("St&atistics\tAlt+4"));
        const QAction* worksheets = actionOf(view, QStringLiteral("&Worksheets"));
        QVERIFY(worksheets != nullptr && worksheets->menu() != nullptr);
        QCOMPARE(worksheets->menu()->actions().size(), 4);

        const QMenu* edit = menuOf(*window, QStringLiteral("&Edit"));
        QVERIFY(edit != nullptr);
        QVERIFY(!actionOf(edit, QStringLiteral("&Copy"))->isEnabled());
        QVERIFY(!actionOf(edit, QStringLiteral("&Paste"))->isEnabled());
        QCOMPARE(actionOf(edit, QStringLiteral("&History"))->menu()->actions().size(), 4);

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
