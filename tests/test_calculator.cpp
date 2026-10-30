#include <QObject>
#include <QTest>

#include "core/calculator.h"
#include "core/commands.h"
#include "core/enginetexts.h"
#include "core/keys.h"
#include "core/view.h"

#include "Header Files/EngineStrings.h"

#include <array>
#include <initializer_list>
#include <optional>
#include <utility>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// tests/test_calculator.cpp 0.3.8

// --- Version history ----------------------------------------------------
// v0.3.8: The Programmer keys (characters, F2 to F8 and F12)
// v0.2.7: The Scientific keys (characters, F3 to F5, Ctrl letters); Scientific commands do nothing in Standard
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

    // The Scientific keys of Windows 7 (windows7-scientific.md part C); none of them acts in the Standard view
    static void theScientificKeysGiveTheirWindows7Commands()
    {
        using chrono_calc::View;
        const std::array<std::pair<char32_t, const char*>, 21> characters = {{
            {U'(', "open_bracket"}, {U')', "close_bracket"}, {U'#', "cube"}, {U'!', "factorial"}, {U';', "integer"},
            {U'i', "inverse"}, {U's', "sin"}, {U'o', "cos"}, {U't', "tan"}, {U'n', "ln"}, {U'l', "log"}, {U'm', "dms"},
            {U'p', "pi"}, {U'v', "scientific_notation"}, {U'x', "exponent"}, {U'e', "exponent"}, {U'y', "power"},
            {U'q', "square"}, {U'd', "modulo"}, {U'r', "reciprocal"}, {U'@', "sqrt"}}};
        for (const auto& [character, command] : characters) {
            QCOMPARE(chrono_calc::commandForCharacter(character, View::Scientific).value_or(""), std::string_view(command));
            QVERIFY(chrono_calc::findCommand(command).has_value());
        }
        for (const char32_t character : U"#!;sotnlmpvxyqdS%h") {
            if (character == U'\0') {
                continue;
            }
            if (character == U'S' || character == U'%' || character == U'h') {
                QVERIFY(!chrono_calc::commandForCharacter(character, View::Scientific).has_value());
            }
            if (character != U'%') {
                QVERIFY(!chrono_calc::commandForCharacter(character, View::Standard).has_value());
            }
        }
        QCOMPARE(chrono_calc::commandForKey(chrono_calc::Key::F3, View::Scientific).value_or(""), "angle.degrees");
        QCOMPARE(chrono_calc::commandForKey(chrono_calc::Key::F4, View::Scientific).value_or(""), "angle.radians");
        QCOMPARE(chrono_calc::commandForKey(chrono_calc::Key::F5, View::Scientific).value_or(""), "angle.grads");
        QVERIFY(!chrono_calc::commandForKey(chrono_calc::Key::F4, View::Standard).has_value());
        const std::array<std::pair<char32_t, const char*>, 7> controls = {{{U'Y', "root"}, {U'G', "power_of_ten"}, {U'B', "cube_root"},
                                                             {U'S', "sinh"}, {U'O', "cosh"}, {U'T', "tanh"}, {U'M', "memory.store"}}};
        for (const auto& [letter, command] : controls) {
            QCOMPARE(chrono_calc::commandForControlLetter(letter, View::Scientific).value_or(""), std::string_view(command));
        }
        QVERIFY(!chrono_calc::commandForControlLetter(U'G', View::Standard).has_value());
    }

    // The Programmer keys of Windows 7 (windows7-programmer.md part C)
    static void theProgrammerKeysGiveTheirWindows7Commands()
    {
        using chrono_calc::Key;
        using chrono_calc::View;
        const std::array<std::pair<char32_t, const char*>, 17> characters = {{
            {U'&', "and"}, {U'|', "or"}, {U'^', "xor"}, {U'~', "not"}, {U'<', "shift_left"}, {U'>', "shift_right"},
            {U'%', "modulo"}, {U'(', "open_bracket"}, {U')', "close_bracket"}, {U'a', "digit.a"}, {U'b', "digit.b"},
            {U'c', "digit.c"}, {U'd', "digit.d"}, {U'e', "digit.e"}, {U'f', "digit.f"}, {U'j', "rotate_left"}, {U'k', "rotate_right"}}};
        for (const auto& [character, command] : characters) {
            QCOMPARE(chrono_calc::commandForCharacter(character, View::Programmer).value_or(""), std::string_view(command));
            QVERIFY(chrono_calc::findCommand(command).has_value());
            QVERIFY(chrono_calc::commandForCharacter(character, View::Standard) != std::optional<std::string_view>(command));
        }
        const std::array<std::pair<Key, const char*>, 8> keys = {{
            {Key::F5, "base.hexadecimal"}, {Key::F6, "base.decimal"}, {Key::F7, "base.octal"}, {Key::F8, "base.binary"},
            {Key::F12, "word.qword"}, {Key::F2, "word.dword"}, {Key::F3, "word.word"}, {Key::F4, "word.byte"}}};
        for (const auto& [key, command] : keys) {
            QCOMPARE(chrono_calc::commandForKey(key, View::Programmer).value_or(""), std::string_view(command));
            QVERIFY(chrono_calc::findCommand(command).has_value());
        }
        QVERIFY(!chrono_calc::commandForCharacter(U's', View::Programmer).has_value());
        QCOMPARE(chrono_calc::commandForCharacter(U'F', View::Programmer).value_or(""), "digit.f");  // Shift+F (Windows 7)
        QVERIFY(!chrono_calc::commandForControlLetter(U'G', View::Programmer).has_value());
        QCOMPARE(chrono_calc::commandForControlLetter(U'M', View::Programmer).value_or(""), "memory.store");
    }

    // A Scientific command does nothing in the Standard view (no button there; its key gives nothing)
    static void aScientificCommandDoesNothingInStandard()
    {
        chrono_calc::Calculator calculator;
        QCOMPARE(text(after(calculator, {"digit.3", "digit.0", "sin", "open_bracket", "inverse"})), QStringLiteral("30"));
        QCOMPARE(text(calculator.expression()), QString());
        QVERIFY(!calculator.isInverse());
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
