#include <QObject>
#include <QString>
#include <QTest>

#include "core/calculator.h"
#include "core/enginetexts.h"

#include <cstddef>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// tests/test_reference_standard.cpp 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: Lines 21 to 36: the keys checked on the Windows computer, the memory at 0, percent after =, Standard again
// v0.1.5: Initial: the 20 Windows 7 Standard view results of tests/reference/windows7-standard.md (task 1.5)

// Each line of parts C to F of tests/reference/windows7-standard.md, measured in the Windows 7 Calculator: the commands (Escape
// first, as there), and the display, expression line and memory mark Windows 7 showed. That computer used a comma as
// the decimal separator, so these calculators do too.
namespace {

struct Line
{
    const char* name;
    std::vector<std::string_view> commands;
    const char* display;
    const char* expression;
    bool memory;
};

std::vector<Line> lines()
{
    return {
        {"1: 2+3*", {"digit.2", "add", "digit.3", "multiply"}, "5", "2 + 3 *", false},
        {"2: 2+3*4=", {"digit.2", "add", "digit.3", "multiply", "digit.4", "equals"}, "20", "", false},
        {"3: 1/0=", {"digit.1", "divide", "digit.0", "equals"}, "Cannot divide by zero", "1 /", false},
        {"4: 0/0=", {"digit.0", "divide", "digit.0", "equals"}, "Result is undefined", "0 /", false},
        {"5: 1 +/- sqrt", {"digit.1", "negate", "sqrt"}, "Invalid input", "sqrt(-1)", false},
        {"6: 5 1/x", {"digit.5", "reciprocal"}, "0,2", "reciproc(5)", false},
        {"7: 9 sqrt", {"digit.9", "sqrt"}, "3", "sqrt(9)", false},
        {"8: 2+3%", {"digit.2", "add", "digit.3", "percent"}, "0,06", "2 + 0,06", false},
        {"9: 1/3=", {"digit.1", "divide", "digit.3", "equals"}, "0,3333333333333333", "", false},
        {"10: 2/3=", {"digit.2", "divide", "digit.3", "equals"}, "0,6666666666666667", "", false},
        {"11: 9999999999999999*9999999999999999=",
         {"digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9",
          "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "multiply", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9",
          "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9",
          "equals"},
         "9,999999999999998e+31", "", false},
        {"12: 0.1+0.2=", {"digit.0", "decimal", "digit.1", "add", "digit.0", "decimal", "digit.2", "equals"}, "0,3", "", false},
        {"13: 5 +/-", {"digit.5", "negate"}, "-5", "", false},
        {"14: 123 backspace", {"digit.1", "digit.2", "digit.3", "backspace"}, "12", "", false},
        {"15: 2+=", {"digit.2", "add", "equals"}, "4", "", false},
        {"16: 2+3===", {"digit.2", "add", "digit.3", "equals", "equals", "equals"}, "11", "", false},
        {"17: 100*5%", {"digit.1", "digit.0", "digit.0", "multiply", "digit.5", "percent"}, "5", "100 * 5", false},
        {"18a: 5 MS (M shows right after MS)", {"digit.5", "memory.store"}, "5", "", true},
        {"18: 5 MS Esc MR", {"digit.5", "memory.store", "clear", "memory.recall"}, "5", "", true},
        // Line 19 started with 5 in memory, left over from line 18: the first three commands put it there
        {"19: 5 M+ 3 M+ MR (memory held 5)", {"digit.5", "memory.store", "clear", "digit.5", "memory.add", "digit.3", "memory.add", "memory.recall"}, "13", "", true},
        // Line 20 showed M only because memory still held 13 from line 19; each line here starts with an empty memory
        {"20: 12+3 CE 4=", {"digit.1", "digit.2", "add", "digit.3", "clear_entry", "digit.4", "equals"}, "16", "", false},
        // Part D: the keys (core/keys gives these commands; the window's own key test sends the real keys)
        {"21: 12+3 Delete", {"digit.1", "digit.2", "add", "digit.3", "clear_entry"}, "0", "12 +", false},
        {"22: 12+3 Escape", {"digit.1", "digit.2", "add", "digit.3", "clear"}, "0", "", false},
        {"23: 5 F9", {"digit.5", "negate"}, "-5", "", false},
        {"24: 2*3 Enter", {"digit.2", "multiply", "digit.3", "equals"}, "6", "", false},
        {"25: 5 Ctrl+M Esc Ctrl+R", {"digit.5", "memory.store", "clear", "memory.recall"}, "5", "", true},
        {"26: then 3 Ctrl+P Ctrl+R", {"digit.5", "memory.store", "clear", "memory.recall", "digit.3", "memory.add", "memory.recall"}, "8", "", true},
        {"27: then 1 Ctrl+Q Ctrl+R",
         {"digit.5", "memory.store", "clear", "memory.recall", "digit.3", "memory.add", "memory.recall", "digit.1", "memory.subtract", "memory.recall"},
         "7", "", true},
        {"28: then Ctrl+L Ctrl+R",
         {"digit.5", "memory.store", "clear", "memory.recall", "digit.3", "memory.add", "memory.recall", "digit.1", "memory.subtract", "memory.recall",
          "memory.clear", "memory.recall"},
         "0", "", false},
        // Part E: the memory at 0 shows no M
        {"29: MC Esc MR", {"memory.clear", "clear", "memory.recall"}, "0", "", false},
        {"30: 0 MS", {"digit.0", "memory.store"}, "0", "", false},
        {"31: 5 MS 0 MS", {"digit.5", "memory.store", "digit.0", "memory.store"}, "0", "", false},
        {"32: MC 5 M+ 5 M- MR", {"memory.clear", "digit.5", "memory.add", "digit.5", "memory.subtract", "memory.recall"}, "0", "", false},
        // Part F: percent right after = takes the result; Standard and Basic again change nothing
        {"33: 2*3=5%", {"digit.2", "multiply", "digit.3", "equals", "digit.5", "percent"}, "0,3", "0,3", false},
        {"34: 2+3=5%", {"digit.2", "add", "digit.3", "equals", "digit.5", "percent"}, "0,25", "0,25", false},
        {"35: 2+3 Alt+1", {"digit.2", "add", "digit.3", "view.standard"}, "3", "2 +", false},
        {"36: 2+3 Ctrl+F4", {"digit.2", "add", "digit.3", "view.basic"}, "3", "2 +", false},
    };
}

}  // namespace

class TestReferenceStandard : public QObject
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
        chrono_calc::Calculator calculator(chrono_calc::Separators{.decimal = L',', .thousands = L'.'});
        for (const std::string_view command : line.commands) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(QString::fromStdWString(calculator.display()), QString::fromUtf8(line.display));
        QCOMPARE(QString::fromStdWString(calculator.expression()), QString::fromUtf8(line.expression));
        QCOMPARE(calculator.hasMemory(), line.memory);
    }
};

QTEST_GUILESS_MAIN(TestReferenceStandard)
#include "test_reference_standard.moc"
