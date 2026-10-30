#include <QObject>
#include <QTest>

#include "ui/mainwindow.h"

// --- Version ------------------------------------------------------------
// tests/test_mainwindow.cpp 0.0.2

// --- Version history ----------------------------------------------------
// v0.0.2: Initial: the window opens with the title Chrono Calc

class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    static void theWindowOpensWithTheTitleChronoCalc()
    {
        chrono_calc::MainWindow window;
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QCOMPARE(window.windowTitle(), QStringLiteral("Chrono Calc"));
    }
};

QTEST_MAIN(TestMainWindow)
#include "test_mainwindow.moc"
