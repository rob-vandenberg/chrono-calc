#include <QObject>
#include <QTest>

#include "core/calculator.h"
#include "core/commands.h"
#include "core/enginetexts.h"
#include "core/keys.h"

#include "Header Files/EngineStrings.h"

#include <cstddef>
#include <initializer_list>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// tests/test_calculator.cpp 0.0.4

// --- Version history ----------------------------------------------------
// v0.0.4: Initial: the engine wrapper, the command registry, the typed keys and the engine texts

namespace {

// Runs the commands in order and returns what the display shows
std::wstring after(chrono_calc::Calculator& calculator, std::initializer_list<std::string_view> commands)
{
    for (const std::string_view command : commands) {
        if (!calculator.run(command)) {
            return L"unknown command " + std::wstring(command.begin(), command.end());
        }
    }
    return calculator.display();
}

QString text(const std::wstring& value)
{
    return QString::fromStdWString(value);
}

}  // namespace

class TestCalculator : public QObject
{
    Q_OBJECT

private slots:
    static void startsAtZero()
    {
        const chrono_calc::Calculator calculator;
        QCOMPARE(text(calculator.display()), QStringLiteral("0"));
        QVERIFY(!calculator.isError());
        QVERIFY(!calculator.hasMemory());
    }

    static void addsTwoNumbers()
    {
        chrono_calc::Calculator calculator;
        QCOMPARE(text(after(calculator, {"digit.2", "add", "digit.3", "equals"})), QStringLiteral("5"));
    }

    // The Standard view works from left to right: 2 + 3 x 4 = 20, not 14
    static void standardWorksLeftToRight()
    {
        chrono_calc::Calculator calculator;
        QCOMPARE(text(after(calculator, {"digit.2", "add", "digit.3", "multiply", "digit.4", "equals"})), QStringLiteral("20"));
    }

    static void theExpressionLineShowsTheOperation()
    {
        chrono_calc::Calculator calculator;
        after(calculator, {"digit.2", "add"});
        const QString expression = text(calculator.expression());
        QVERIFY2(expression.contains(QLatin1Char('2')) && expression.contains(QLatin1Char('+')), qPrintable(expression));
    }

    static void divisionByZeroIsAnErrorWithAText()
    {
        chrono_calc::Calculator calculator;
        const std::wstring shown = after(calculator, {"digit.1", "divide", "digit.0", "equals"});
        QVERIFY(calculator.isError());
        QCOMPARE(text(shown), QStringLiteral("Cannot divide by zero"));
        QCOMPARE(text(after(calculator, {"clear"})), QStringLiteral("0"));
        QVERIFY(!calculator.isError());
    }

    // One memory, as in Windows 7: MS replaces it, MR brings it back, MC empties it
    static void memoryStoreReplacesTheOneMemory()
    {
        chrono_calc::Calculator calculator;
        after(calculator, {"digit.5", "memory.store", "clear", "digit.7", "memory.store", "clear", "memory.recall"});
        QCOMPARE(text(calculator.display()), QStringLiteral("7"));
        QVERIFY(calculator.hasMemory());
        QCOMPARE(calculator.memoryPlaces(), std::size_t{1});
        after(calculator, {"memory.clear"});
        QVERIFY(!calculator.hasMemory());
    }

    static void memoryAddAndSubtract()
    {
        chrono_calc::Calculator calculator;
        QCOMPARE(text(after(calculator, {"digit.5", "memory.add", "digit.3", "memory.add", "clear", "memory.recall"})), QStringLiteral("8"));
        QCOMPARE(text(after(calculator, {"digit.2", "memory.subtract", "clear", "memory.recall"})), QStringLiteral("6"));
    }

    static void anUnknownCommandDoesNothing()
    {
        chrono_calc::Calculator calculator;
        after(calculator, {"digit.4"});
        QVERIFY(!calculator.run("no.such.command"));
        QCOMPARE(text(calculator.display()), QStringLiteral("4"));
    }

    // Every command in the registry runs; none crashes, none is unknown
    static void everyRegisteredCommandRuns()
    {
        chrono_calc::Calculator calculator;
        for (const std::string_view id : chrono_calc::commandIds()) {
            QVERIFY2(calculator.run(id), id.data());
        }
    }

    // Every typed character gives a command that exists in the registry
    static void everyTypedCharacterGivesARegisteredCommand()
    {
        for (const char32_t character : U"0123456789.,+-*/=\r\n@%rR") {
            if (character == U'\0') {
                continue;
            }
            const std::string_view command = chrono_calc::commandForCharacter(character).value_or("");
            QVERIFY(!command.empty());
            QVERIFY2(chrono_calc::findCommand(command).has_value(), command.data());
        }
        QVERIFY(!chrono_calc::commandForCharacter(U'x').has_value());
    }

    // The engine stores a key to a temporary string when it looks up a text number it was given no text for
    // (CalcEngine.h, GetString(int)); so every text the engine can ask for has one. Only the empty string stays empty.
    static void everyEngineTextHasAText()
    {
        chrono_calc::EngineTexts texts;
        for (const std::wstring_view id : g_sids) {
            if (id == SIDS_EMPTY_STRING) {
                continue;
            }
            QVERIFY2(!texts.GetCEngineString(id).empty(), qPrintable(QString::fromStdWString(std::wstring(id))));
        }
    }
};

QTEST_GUILESS_MAIN(TestCalculator)
#include "test_calculator.moc"
