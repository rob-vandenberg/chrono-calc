#include <QObject>
#include <QString>
#include <QTest>

#include "core/calculator.h"
#include "core/enginetexts.h"
#include "core/view.h"

#include <cstddef>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// tests/test_reference_programmer.cpp 0.3.8

// --- Version history ----------------------------------------------------
// v0.3.8: Initial: the Windows 7 Programmer view results of tests/reference/windows7-programmer.md (M3)

// Each line of parts B to D of tests/reference/windows7-programmer.md, measured in the Windows 7 Calculator: the
// commands (Escape first, as there, in the Programmer view, Dec and Qword) and the display Windows 7 showed. The
// expression line stays empty in this view.
namespace {

struct Line
{
    const char* name;
    std::vector<std::string_view> commands;
    const char* display;
};

std::vector<Line> lines()
{
    return {
        {"1: 12+3*4=", {"digit.1", "digit.2", "add", "digit.3", "multiply", "digit.4", "equals"}, "24"},
        {"2: 2+3*", {"digit.2", "add", "digit.3", "multiply"}, "3"},
        {"3: 7/2=", {"digit.7", "divide", "digit.2", "equals"}, "3"},
        {"4: 17 Mod 5 =", {"digit.1", "digit.7", "modulo", "digit.5", "equals"}, "2"},
        {"5: 12 And 10 =", {"digit.1", "digit.2", "and", "digit.1", "digit.0", "equals"}, "8"},
        {"6: 12 Or 3 =", {"digit.1", "digit.2", "or", "digit.3", "equals"}, "15"},
        {"7: 12 Xor 10 =", {"digit.1", "digit.2", "xor", "digit.1", "digit.0", "equals"}, "6"},
        {"8: 5 Not", {"digit.5", "not"}, "-6"},
        {"9: 1 Lsh 4 =", {"digit.1", "shift_left", "digit.4", "equals"}, "16"},
        {"10: 256 Rsh 2 =", {"digit.2", "digit.5", "digit.6", "shift_right", "digit.2", "equals"}, "64"},
        {"11a: Byte Hex 81 RoL", {"word.byte", "base.hexadecimal", "digit.8", "digit.1", "rotate_left"}, "3"},
        {"11b: then RoR", {"word.byte", "base.hexadecimal", "digit.8", "digit.1", "rotate_left", "rotate_right"}, "81"},
        {"11c: then RoR again", {"word.byte", "base.hexadecimal", "digit.8", "digit.1", "rotate_left", "rotate_right", "rotate_right"}, "C0"},
        {"11d: C0 in Dec", {"word.byte", "base.hexadecimal", "digit.8", "digit.1", "rotate_left", "rotate_right", "rotate_right", "base.decimal"}, "-64"},
        {"12: Qword 1 RoR", {"digit.1", "rotate_right"}, "-9223372036854775808"},
        {"13: Hex FF+1=", {"base.hexadecimal", "digit.f", "digit.f", "add", "digit.1", "equals"}, "100"},
        {"14: Hex A*B=", {"base.hexadecimal", "digit.a", "multiply", "digit.b", "equals"}, "6E"},
        {"15: Oct 777+1=", {"base.octal", "digit.7", "digit.7", "digit.7", "add", "digit.1", "equals"}, "1000"},
        {"16: Bin 1011+1=", {"base.binary", "digit.1", "digit.0", "digit.1", "digit.1", "add", "digit.1", "equals"}, "1100"},
        {"17a: 255 in Hex", {"digit.2", "digit.5", "digit.5", "base.hexadecimal"}, "FF"},
        {"17b: 255 in Oct", {"digit.2", "digit.5", "digit.5", "base.octal"}, "377"},
        {"17c: 255 in Bin", {"digit.2", "digit.5", "digit.5", "base.binary"}, "11111111"},
        {"18a: 5 +/-", {"digit.5", "negate"}, "-5"},
        {"18b: 5 +/- in Hex", {"digit.5", "negate", "base.hexadecimal"}, "FFFFFFFFFFFFFFFB"},
        {"18c: then Dec again", {"digit.5", "negate", "base.hexadecimal", "base.binary", "base.decimal"}, "-5"},
        {"19: Byte 5 +/- Hex", {"word.byte", "digit.5", "negate", "base.hexadecimal"}, "FB"},
        {"20: 1/0=", {"digit.1", "divide", "digit.0", "equals"}, "Cannot divide by zero"},
        {"21a: 0 Mod 0 =", {"digit.0", "modulo", "digit.0", "equals"}, "Result is undefined"},
        {"21b: 5 Mod 0 =", {"digit.5", "modulo", "digit.0", "equals"}, "Result is undefined"},
        {"22: (2+3)*4=", {"open_bracket", "digit.2", "add", "digit.3", "close_bracket", "multiply", "digit.4", "equals"}, "20"},
        {"23: 1/x is greyed: 5 1/x does nothing", {"digit.5", "reciprocal"}, "5"},
        {"24: 9223372036854775807+1=", {"digit.9", "digit.2", "digit.2", "digit.3", "digit.3", "digit.7", "digit.2", "digit.0", "digit.3", "digit.6", "digit.8", "digit.5", "digit.4", "digit.7", "digit.7", "digit.5", "digit.8", "digit.0", "digit.7", "add", "digit.1", "equals"}, "-9223372036854775808"},
        {"25: 18446744073709551615 typed (the last digit is refused)", {"digit.1", "digit.8", "digit.4", "digit.4", "digit.6", "digit.7", "digit.4", "digit.4", "digit.0", "digit.7", "digit.3", "digit.7", "digit.0", "digit.9", "digit.5", "digit.5", "digit.1", "digit.6", "digit.1", "digit.5"}, "1844674407370955161"},
        {"26: Hex 16 F's in Dec", {"base.hexadecimal", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "base.decimal"}, "-1"},
        {"27: 2*3= 5 % (Mod waits)", {"digit.2", "multiply", "digit.3", "equals", "digit.5", "modulo"}, "5"},
        {"28: 10 Int (no Int)", {"digit.1", "digit.0", "integer"}, "10"},
        {"29a: 1 Lsh 64 =", {"digit.1", "shift_left", "digit.6", "digit.4", "equals"}, "Result not defined"},
        {"29b: 1 Lsh 65 =", {"digit.1", "shift_left", "digit.6", "digit.5", "equals"}, "Result not defined"},
        {"29c: 1 Lsh 63 =", {"digit.1", "shift_left", "digit.6", "digit.3", "equals"}, "-9223372036854775808"},
        {"30: Byte 255+1= (255 types as 25)", {"word.byte", "digit.2", "digit.5", "digit.5", "add", "digit.1", "equals"}, "26"},
        {"B4a: 2,5 types as 25", {"digit.2", "decimal", "digit.5"}, "25"},
        {"B6a: Byte 127+1=", {"word.byte", "digit.1", "digit.2", "digit.7", "add", "digit.1", "equals"}, "-128"},
        {"B6b: Byte 200+100=", {"word.byte", "digit.2", "digit.0", "digit.0", "add", "digit.1", "digit.0", "digit.0", "equals"}, "120"},
        {"B6c: Word 65535+1=", {"word.word", "digit.6", "digit.5", "digit.5", "digit.3", "digit.5", "add", "digit.1", "equals"}, "6554"},
        {"B7a: 300 then Byte", {"digit.3", "digit.0", "digit.0", "word.byte"}, "44"},
        {"B7b: then Qword (the bits do not come back)", {"digit.3", "digit.0", "digit.0", "word.byte", "word.qword"}, "44"},
        {"B8a: Bin Byte, 9 ones", {"word.byte", "base.binary", "digit.1", "digit.1", "digit.1", "digit.1", "digit.1", "digit.1", "digit.1", "digit.1", "digit.1"}, "11111111"},
        {"B8b: Hex Qword, 17 F's", {"base.hexadecimal", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f", "digit.f"}, "FFFFFFFFFFFFFFFF"},
        {"B3: Dec 0, bit 3", {"bit.3"}, "8"},
        {"B3b: bit 3 twice", {"bit.3", "bit.3"}, "0"},
        {"C: 12 Not via ~ at once", {"digit.1", "digit.2", "not"}, "-13"},
        {"F1a: -1 MS, then Standard MR", {"digit.1", "negate", "memory.store", "view.standard", "memory.recall"}, "1,844674407370955e+19"},
        {"F1b: Byte -1 MS, then Standard MR", {"word.byte", "digit.1", "negate", "memory.store", "view.standard", "memory.recall"}, "255"},
        {"F1c: Hex FF MS, then Standard MR", {"base.hexadecimal", "digit.f", "digit.f", "memory.store", "view.standard", "memory.recall"}, "255"},
        {"F2a: Standard 2,5 MS, then Programmer MR", {"view.standard", "digit.2", "decimal", "digit.5", "memory.store", "view.programmer", "memory.recall"}, "2"},
        {"F2b: then Standard MR again", {"view.standard", "digit.2", "decimal", "digit.5", "memory.store", "view.programmer", "memory.recall", "view.standard", "memory.recall"}, "2,5"},
    };
}

constexpr chrono_calc::Separators commaSeparators{.decimal = L',', .thousands = L'.'};

std::string_view digitCommand(char digit)
{
    static const std::vector<std::string_view> digits = {"digit.0", "digit.1", "digit.2", "digit.3", "digit.4",
                                                         "digit.5", "digit.6", "digit.7", "digit.8", "digit.9"};
    return digits.at(static_cast<std::size_t>(digit - '0'));
}

}  // namespace

class TestReferenceProgrammer : public QObject
{
    Q_OBJECT

private slots:
    static void windows7ShowsTheSame_data()
    {
        QTest::addColumn<int>("index");
        const std::vector<Line> all = lines();
        for (std::size_t index = 0; index < all.size(); ++index) {
            QTest::newRow(all[index].name) << static_cast<int>(index);
        }
    }

    static void windows7ShowsTheSame()
    {
        QFETCH(int, index);  // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        const Line line = lines()[static_cast<std::size_t>(index)];
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("view.programmer"));
        for (const std::string_view command : line.commands) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(QString::fromStdWString(calculator.display()), QString::fromUtf8(line.display));
        QCOMPARE(QString::fromStdWString(calculator.expression()), QString());
    }

    // Part B: a view change clears the number, keeps the memory, the base and the word size
    static void aViewChangeKeepsTheBaseAndTheWordSize()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        for (const std::string_view command : {"digit.5", "memory.store", "view.programmer", "base.hexadecimal", "word.byte",
                                               "digit.f", "view.standard", "view.programmer"}) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("0"));
        QVERIFY(calculator.hasMemory());
        QCOMPARE(calculator.numberBase(), chrono_calc::NumberBase::Hexadecimal);
        QCOMPARE(calculator.wordSize(), chrono_calc::WordSize::Byte);
        QVERIFY(calculator.isChosen("base.hexadecimal"));
        QVERIFY(calculator.isChosen("word.byte"));
        QVERIFY(calculator.run("memory.recall"));
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("5"));
    }

    // Every view calculates with its own precision, whatever view came before: the number library's constants are
    // shared by the engines and each view sets its own again (found in review: wrong results after a view change)
    static void everyViewKeepsItsOwnPrecision()
    {
        const auto result = [](std::initializer_list<std::string_view> commands) {
            chrono_calc::Calculator calculator(commaSeparators);
            for (const std::string_view command : commands) {
                calculator.run(command);
            }
            return QString::fromStdWString(calculator.display());
        };
        // 64-bit integers right after the view change: 9223372036854775807 - 9223372036854775806 = 1
        std::vector<std::string_view> subtraction = {"view.programmer"};
        for (const char digit : std::string("9223372036854775807")) {
            subtraction.emplace_back(digitCommand(digit));
        }
        subtraction.emplace_back("subtract");
        for (const char digit : std::string("9223372036854775806")) {
            subtraction.emplace_back(digitCommand(digit));
        }
        subtraction.emplace_back("equals");
        chrono_calc::Calculator programmer(commaSeparators);
        for (const std::string_view command : subtraction) {
            programmer.run(command);
        }
        QCOMPARE(QString::fromStdWString(programmer.display()), QStringLiteral("1"));
        // Standard after a visit to Programmer gives what a fresh Standard gives
        const std::initializer_list<std::string_view> rootSquared = {"digit.2", "sqrt", "multiply", "equals", "subtract", "digit.2", "equals"};
        const QString fresh = result(rootSquared);
        chrono_calc::Calculator visited(commaSeparators);
        for (const std::string_view command : {"view.programmer", "base.decimal", "view.standard"}) {
            visited.run(command);
        }
        (void)visited.bits();
        for (const std::string_view command : rootSquared) {
            visited.run(command);
        }
        QCOMPARE(QString::fromStdWString(visited.display()), fresh);
        // Scientific's 32 digits, also after Programmer: 1 - 0,99999999999999999999 = 0,00000000000000000001
        QCOMPARE(result({"view.programmer", "base.hexadecimal", "view.scientific", "digit.1", "subtract", "digit.0", "decimal", "digit.9",
                         "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9",
                         "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "equals"}),
                 QStringLiteral("0,00000000000000000001"));
    }

    // The bit display shows the engine's bits of the current value, in two's complement for a negative number
    static void theBitsAreTheNumbersBits()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        for (const std::string_view command : {"view.programmer", "digit.2", "digit.5", "digit.5"}) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(calculator.bits(), std::string(56, '0') + std::string(8, '1'));
        QVERIFY(calculator.run("negate"));
        QCOMPARE(calculator.bits(), std::string(56, '1') + "00000001");
        QVERIFY(calculator.run("word.byte"));
        QCOMPARE(calculator.wordBits(), 8);
        QCOMPARE(calculator.bits().substr(56), std::string("00000001"));
    }
};

QTEST_GUILESS_MAIN(TestReferenceProgrammer)
#include "test_reference_programmer.moc"
