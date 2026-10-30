#include <QObject>
#include <QTest>

#include <string_view>

#include "core/version.h"

// --- Version ------------------------------------------------------------
// tests/test_version.cpp 0.0.2

// --- Version history ----------------------------------------------------
// v0.0.2: Initial: the program's version text is the project version of CMakeLists.txt

class TestVersion : public QObject
{
    Q_OBJECT

private slots:
    static void theVersionTextIsTheProjectVersion()
    {
        QCOMPARE(chrono_calc::versionText(), std::string_view(CHRONO_CALC_PROJECT_VERSION));
    }
};

QTEST_GUILESS_MAIN(TestVersion)
#include "test_version.moc"
