#include <QObject>
#include <QTest>

#include "core/calculator.h"
#include "core/commands.h"
#include "core/enginetexts.h"
#include "core/keys.h"

#include "Header Files/EngineStrings.h"

#include <initializer_list>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// tests/test_calculator.cpp 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: The keys without a character and the Ctrl keys of the memory; view.basic changes nothing
// v0.1.5: The one memory is the engine's own register; the start value comes from the engine
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
        chrono_calc::Calculator calculator;
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
        QCOMPARE(text(calculator.expression()), QStringLiteral("2 +"));
        // Another operator right after one replaces it
        after(calculator, {"multiply"});
        QCOMPARE(text(calculator.expression()), QStringLiteral("2 *"));
        QCOMPARE(text(after(calculator, {"digit.3", "equals"})), QStringLiteral("6"));
    }

    // While an error shows, CE clears everything, as C does: nothing of the old line or the old operator stays
    static void clearEntryAfterAnErrorClearsAll()
    {
        chrono_calc::Calculator calculator;
        after(calculator, {"digit.1", "divide", "digit.0", "equals", "clear_entry", "digit.5", "add"});
        QCOMPARE(text(calculator.expression()), QStringLiteral("5 +"));
        QCOMPARE(text(after(calculator, {"digit.2", "multiply", "digit.0", "reciprocal", "clear_entry", "digit.5", "percent"})),
                 QStringLiteral("0"));
        QCOMPARE(text(calculator.expression()), QStringLiteral("0"));
    }

    // The Windows 7 percent after * uses the memory register for a moment; the user's memory stays as it was
    static void percentAfterMultiplyKeepsTheMemory()
    {
        chrono_calc::Calculator calculator;
        QCOMPARE(text(after(calculator, {"digit.7", "memory.store", "clear", "digit.2", "multiply", "digit.3", "percent"})),
                 QStringLiteral("0.06"));
        QCOMPARE(text(calculator.expression()), QStringLiteral("2 * 0.06"));
        // Backspace changes nothing on a percent result
        QCOMPARE(text(after(calculator, {"backspace"})), QStringLiteral("0.06"));
        QCOMPARE(text(calculator.expression()), QStringLiteral("2 * 0.06"));
        QCOMPARE(text(after(calculator, {"equals"})), QStringLiteral("0.12"));
        QCOMPARE(text(after(calculator, {"memory.recall"})), QStringLiteral("7"));
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
        QCOMPARE(text(after(calculator, {"memory.subtract", "memory.recall"})), QStringLiteral("0"));
        QVERIFY2(!calculator.hasMemory(), "7 - 7 leaves 0 in memory: no M");
        after(calculator, {"digit.4", "memory.store", "memory.clear"});
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

    // The keys without a character and the Ctrl letters give the commands of Windows 7
    static void everyKeyGivesItsWindows7Command()
    {
        using chrono_calc::Key;
        QCOMPARE(chrono_calc::commandForKey(Key::Escape).value_or(""), "clear");
        QCOMPARE(chrono_calc::commandForKey(Key::Delete).value_or(""), "clear_entry");
        QCOMPARE(chrono_calc::commandForKey(Key::Backspace).value_or(""), "backspace");
        QCOMPARE(chrono_calc::commandForKey(Key::Enter).value_or(""), "equals");
        QCOMPARE(chrono_calc::commandForKey(Key::F9).value_or(""), "negate");
        QCOMPARE(chrono_calc::commandForControlLetter(U'l').value_or(""), "memory.clear");
        QCOMPARE(chrono_calc::commandForControlLetter(U'R').value_or(""), "memory.recall");
        QCOMPARE(chrono_calc::commandForControlLetter(U'M').value_or(""), "memory.store");
        QCOMPARE(chrono_calc::commandForControlLetter(U'p').value_or(""), "memory.add");
        QCOMPARE(chrono_calc::commandForControlLetter(U'Q').value_or(""), "memory.subtract");
        QVERIFY(!chrono_calc::commandForControlLetter(U'x').has_value());
        for (const Key key : {Key::Escape, Key::Delete, Key::Backspace, Key::Enter, Key::F9}) {
            QVERIFY(chrono_calc::findCommand(chrono_calc::commandForKey(key).value_or("")).has_value());
        }
    }

    // A percent right after = takes the result (Windows 7, D-019); not measured, worked out by the same rule:
    // the memory stays as it was, a digit after = starts a new number, and % alone takes the result itself
    static void aPercentAfterEqualsLeavesTheRestAsItWas()
    {
        chrono_calc::Calculator calculator({.decimal = L',', .thousands = L'.'});
        QCOMPARE(text(after(calculator, {"digit.7", "memory.store", "clear", "digit.2", "multiply", "digit.3", "equals", "digit.5", "percent"})),
                 QStringLiteral("0,3"));
        QVERIFY(calculator.hasMemory());
        QCOMPARE(text(after(calculator, {"memory.recall"})), QStringLiteral("7"));
        QCOMPARE(text(after(calculator, {"clear", "digit.2", "multiply", "digit.3", "equals", "digit.7"})), QStringLiteral("7"));
        QCOMPARE(text(after(calculator, {"clear", "digit.2", "multiply", "digit.3", "equals", "percent"})), QStringLiteral("0,36"));
    }

    // View > Basic only hides a side panel: the number, the expression line and the memory stay
    static void basicChangesNothing()
    {
        chrono_calc::Calculator calculator;
        after(calculator, {"digit.5", "memory.store", "digit.2", "add", "digit.3"});
        QVERIFY(calculator.run("view.basic"));
        QCOMPARE(text(calculator.display()), QStringLiteral("3"));
        QCOMPARE(text(calculator.expression()), QStringLiteral("2 +"));
        QVERIFY(calculator.hasMemory());
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
