#include <QObject>
#include <QString>
#include <QTest>

#include "core/calculator.h"
#include "core/enginetexts.h"
#include "core/view.h"

#include <cstddef>
#include <initializer_list>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// tests/test_reference_scientific.cpp 0.2.7

// --- Version history ----------------------------------------------------
// v0.2.7: Initial: the Windows 7 Scientific view results of tests/reference/windows7-scientific.md (M2)

// Each line of part D of tests/reference/windows7-scientific.md, measured in the Windows 7 Calculator: the commands
// (Escape first, as there, in the Scientific view, Degrees), and the display, the expression line, the bracket counter
// and Inv that Windows 7 showed. That computer used a comma as the decimal separator, so these calculators do too.
namespace {

struct Line
{
    const char* name;
    std::vector<std::string_view> commands;
    const char* display;
    const char* expression;
    unsigned int openBrackets;
    bool inverse;
};

std::vector<Line> lines()
{
    return {
        {"1: 2+3*4=", {"digit.2", "add", "digit.3", "multiply", "digit.4", "equals"}, "14", "", 0, false},
        {"2: 2+3*", {"digit.2", "add", "digit.3", "multiply"}, "3", "2 + 3 *", 0, false},
        {"3: 30 sin", {"digit.3", "digit.0", "sin"}, "0,5", "sind(30)", 0, false},
        {"4: 45 tan", {"digit.4", "digit.5", "tan"}, "1", "tand(45)", 0, false},
        {"5: 90 tan", {"digit.9", "digit.0", "tan"}, "Invalid input", "tand(90)", 0, false},
        {"6: 1 Inv sin", {"digit.1", "inverse", "sin"}, "90", "asind(1)", 0, false},
        {"7: 0,5 Inv cos", {"digit.0", "decimal", "digit.5", "inverse", "cos"}, "60", "acosd(0,5)", 0, false},
        {"8: Radians pi sin", {"angle.radians", "pi", "sin"}, "0", "sinr(3,1415926535897932384626433832795)", 0, false},
        {"9: Radians 1 cos", {"angle.radians", "digit.1", "cos"}, "0,54030230586813971740093660744298", "cosr(1)", 0, false},
        {"10: Grads 100 sin", {"angle.grads", "digit.1", "digit.0", "digit.0", "sin"}, "1", "sing(100)", 0, false},
        {"11: 2 x^y 10 =", {"digit.2", "power", "digit.1", "digit.0", "equals"}, "1024", "", 0, false},
        {"12: 27 y-root 3 =", {"digit.2", "digit.7", "root", "digit.3", "equals"}, "3", "", 0, false},
        {"12b: 27 Inv x^y 3 = (Inv has no effect and stays on)", {"digit.2", "digit.7", "inverse", "power", "digit.3", "equals"}, "19683", "", 0, true},
        {"13: 5 n!", {"digit.5", "factorial"}, "120", "fact(5)", 0, false},
        {"14: 0,5 n!", {"digit.0", "decimal", "digit.5", "factorial"}, "0,88622692545275801364908374167057", "fact(0,5)", 0, false},
        {"15: 171 n!", {"digit.1", "digit.7", "digit.1", "factorial"}, "1,2410180702176678234248405241031e+309", "fact(171)", 0, false},
        {"16: 100 log", {"digit.1", "digit.0", "digit.0", "log"}, "2", "log(100)", 0, false},
        {"17: 10 ln", {"digit.1", "digit.0", "ln"}, "2,3025850929940456840179914546844", "ln(10)", 0, false},
        {"18: 1 Inv ln", {"digit.1", "inverse", "ln"}, "2,7182818284590452353602874713527", "powe(1)", 0, false},
        {"19: 3 Inv log (no inverse form, Inv stays on)", {"digit.3", "inverse", "log"}, "0,47712125471966243729502790325512", "log(3)", 0, true},
        {"19b: 3 10^x", {"digit.3", "power_of_ten"}, "1000", "powten(3)", 0, false},
        {"20a: 4 x^2", {"digit.4", "square"}, "16", "sqr(4)", 0, false},
        {"20b: 4 x^3", {"digit.4", "cube"}, "64", "cube(4)", 0, false},
        {"20c: 27 cube root", {"digit.2", "digit.7", "cube_root"}, "3", "cuberoot(27)", 0, false},
        {"20d: 27 Inv x^3", {"digit.2", "digit.7", "inverse", "cube"}, "19683", "cube(27)", 0, true},
        {"21: 1 Exp 3 =", {"digit.1", "exponent", "digit.3", "equals"}, "1000", "", 0, false},
        {"22: 1 Exp 3 +/- =", {"digit.1", "exponent", "digit.3", "negate", "equals"}, "0,001", "", 0, false},
        {"23a: 1234 F-E", {"digit.1", "digit.2", "digit.3", "digit.4", "scientific_notation"}, "1,234e+3", "", 0, false},
        {"23b: then 56789", {"digit.1", "digit.2", "digit.3", "digit.4", "scientific_notation", "digit.5", "digit.6", "digit.7", "digit.8", "digit.9"}, "56789", "", 0, false},
        {"23c: 12345 F-E", {"digit.1", "digit.2", "digit.3", "digit.4", "digit.5", "scientific_notation"}, "1,2345e+4", "", 0, false},
        {"24a: 12,5 dms", {"digit.1", "digit.2", "decimal", "digit.5", "dms"}, "12,3", "dms(12,5)", 0, false},
        {"24b: 12,3 Inv dms", {"digit.1", "digit.2", "decimal", "digit.3", "inverse", "dms"}, "12,5", "degrees(12,3)", 0, false},
        {"25: 17 Mod 5 =", {"digit.1", "digit.7", "modulo", "digit.5", "equals"}, "2", "", 0, false},
        {"26a: 3,7 Int", {"digit.3", "decimal", "digit.7", "integer"}, "3", "Int(3,7)", 0, false},
        {"26b: 3,7 Inv Int", {"digit.3", "decimal", "digit.7", "inverse", "integer"}, "0,7", "frac(3,7)", 0, false},
        {"27: (2+3)*4=", {"open_bracket", "digit.2", "add", "digit.3", "close_bracket", "multiply", "digit.4", "equals"}, "20", "", 0, false},
        {"28: (2+3", {"open_bracket", "digit.2", "add", "digit.3"}, "3", "(2 +", 1, false},
        {"29: ((2", {"open_bracket", "open_bracket", "digit.2"}, "2", "((", 2, false},
        {"30: 1/3=", {"digit.1", "divide", "digit.3", "equals"}, "0,33333333333333333333333333333333", "", 0, false},
        {"31: 2 sqrt", {"digit.2", "sqrt"}, "1,4142135623730950488016887242097", "sqrt(2)", 0, false},
        {"32: pi", {"pi"}, "3,1415926535897932384626433832795", "", 0, false},
        {"33a: 1 sinh", {"digit.1", "sinh"}, "1,1752011936438014568823818505956", "sinh(1)", 0, false},
        {"33b: 1 Inv sinh", {"digit.1", "inverse", "sinh"}, "0,88137358701954302523260932497979", "asinh(1)", 0, false},
        {"34: 0 log", {"digit.0", "log"}, "Invalid input", "log(0)", 0, false},
        {"35: 1 +/- ln", {"digit.1", "negate", "ln"}, "Invalid input", "ln(-1)", 0, false},
        {"36: 1/0=", {"digit.1", "divide", "digit.0", "equals"}, "Cannot divide by zero", "1 /", 0, false},
        {"37: 2+3% (% does nothing)", {"digit.2", "add", "digit.3", "percent"}, "3", "2 +", 0, false},
        {"38: 10 x^y 400 =", {"digit.1", "digit.0", "power", "digit.4", "digit.0", "digit.0", "equals"}, "1,e+400", "", 0, false},
        {"39: 1 Exp 400 =", {"digit.1", "exponent", "digit.4", "digit.0", "digit.0", "equals"}, "1,e+400", "", 0, false},
        {"40: 34 nines (32 digits at most)", {"digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9", "digit.9"}, "99999999999999999999999999999999", "", 0, false},
        {"41: 2 x^y 0,5 =", {"digit.2", "power", "digit.0", "decimal", "digit.5", "equals"}, "1,4142135623730950488016887242097", "", 0, false},
        {"42: 0 1/x", {"digit.0", "reciprocal"}, "Cannot divide by zero", "reciproc(0)", 0, false},
        {"43: 5 sin cos", {"digit.5", "sin", "cos"}, "0,9999988430444535451973704698379", "cosd(sind(5))", 0, false},
        {"44: 2+3 sin =", {"digit.2", "add", "digit.3", "sin", "equals"}, "2,0523359562429438327221186296091", "", 0, false},
        {"45: 2,5 +/- n!", {"digit.2", "decimal", "digit.5", "negate", "factorial"}, "2,3632718012073547030642233111215", "fact(-2,5)", 0, false},
        {"46: Inv then C turns Inv off", {"inverse", "clear"}, "0", "", 0, false},
        {"47: Inv then CE turns Inv off", {"inverse", "clear_entry"}, "0", "", 0, false},
        {"48: F-E then CE clears the number", {"digit.5", "scientific_notation", "clear_entry"}, "0", "", 0, false},
        {"50: F-E 1/0=", {"scientific_notation", "digit.1", "divide", "digit.0", "equals"}, "Cannot divide by zero", "1,e+0 /", 0, false},
        {"51: 1/0= MS (ignored) C MR", {"digit.1", "divide", "digit.0", "equals", "memory.store", "clear", "memory.recall"}, "0", "", 0, false},
        {"52: 1/0= Inv (ignored)", {"digit.1", "divide", "digit.0", "equals", "inverse"}, "Cannot divide by zero", "1 /", 0, false},
        {"53: 1/0= ( (ignored, no counter)", {"digit.1", "divide", "digit.0", "equals", "open_bracket"}, "Cannot divide by zero", "1 /", 0, false},
        {"54: 1/0= sin (ignored)", {"digit.1", "divide", "digit.0", "equals", "sin"}, "Cannot divide by zero", "1 /", 0, false},
    };
}

constexpr chrono_calc::Separators commaSeparators{.decimal = L',', .thousands = L'.'};

}  // namespace

class TestReferenceScientific : public QObject
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
        QVERIFY(calculator.run("view.scientific"));
        for (const std::string_view command : line.commands) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(QString::fromStdWString(calculator.display()), QString::fromUtf8(line.display));
        QCOMPARE(QString::fromStdWString(calculator.expression()), QString::fromUtf8(line.expression));
        QCOMPARE(calculator.openBrackets(), line.openBrackets);
        QCOMPARE(calculator.isInverse(), line.inverse);
    }

    // Part B: a view change clears the display and the expression line and keeps the memory
    static void aViewChangeClearsTheNumberAndKeepsTheMemory()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        for (const std::string_view command : {"digit.5", "memory.store", "clear", "digit.1", "digit.2", "add", "digit.3"}) {
            QVERIFY(calculator.run(command));
        }
        QVERIFY(calculator.run("view.scientific"));
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("0"));
        QCOMPARE(QString::fromStdWString(calculator.expression()), QString());
        QVERIFY(calculator.hasMemory());
        QVERIFY(calculator.run("view.standard"));
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("0"));
        QVERIFY(calculator.hasMemory());
        QVERIFY(calculator.run("memory.recall"));
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("5"));
    }

    // Part B: the angle unit, Inv and F-E do not survive a view change (Scientific starts in Degrees)
    static void aViewChangeStartsScientificInDegrees()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        for (const std::string_view command : {"view.scientific", "angle.radians", "inverse", "scientific_notation", "view.standard", "view.scientific"}) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(calculator.angleUnit(), chrono_calc::AngleUnit::Degrees);
        QVERIFY(!calculator.isInverse());
        QVERIFY(!calculator.isScientificNotation());
        for (const std::string_view command : {"digit.3", "digit.0", "sin"}) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("0,5"));
    }

    // F-E: CE keeps it on, C turns it off (Windows 7, part D)
    static void clearEntryKeepsFixedExponentAndClearEndsIt()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        for (const std::string_view command : {"view.scientific", "digit.5", "scientific_notation", "clear_entry"}) {
            QVERIFY(calculator.run(command));
        }
        QVERIFY(calculator.isScientificNotation());
        QVERIFY(calculator.run("clear"));
        QVERIFY(!calculator.isScientificNotation());
        for (const std::string_view command : {"digit.1", "digit.2", "digit.3", "digit.4", "equals"}) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("1234"));
    }

    // While an error shows the engine takes only C and CE: the switches do not change either, so the angle unit and
    // F-E shown are always the engine's (found in review: a unit or F-E changed during an error gave wrong results)
    static void anErrorKeepsTheSwitchesInStepWithTheEngine()
    {
        const auto result = [](std::initializer_list<std::string_view> commands) {
            chrono_calc::Calculator calculator(commaSeparators);
            for (const std::string_view command : commands) {
                calculator.run(command);
            }
            return QString::fromStdWString(calculator.display());
        };
        // A view change after an error in Radians: Scientific starts in Degrees, so 90 sin is 1
        QCOMPARE(result({"view.scientific", "angle.radians", "digit.1", "divide", "digit.0", "equals", "view.standard", "view.scientific",
                         "digit.9", "digit.0", "sin"}),
                 QStringLiteral("1"));
        // Radians during an error is not taken: still Degrees after C
        chrono_calc::Calculator calculator(commaSeparators);
        for (const std::string_view command : {"view.scientific", "digit.1", "divide", "digit.0", "equals", "angle.radians", "inverse"}) {
            calculator.run(command);
        }
        QCOMPARE(calculator.angleUnit(), chrono_calc::AngleUnit::Degrees);
        QVERIFY(!calculator.isInverse());
        for (const std::string_view command : {"clear", "digit.9", "digit.0", "sin"}) {
            calculator.run(command);
        }
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("1"));
        // C during an error turns F-E off in the engine as well: 2+3= shows 5
        QCOMPARE(result({"view.scientific", "scientific_notation", "digit.1", "divide", "digit.0", "equals", "clear", "digit.2", "add",
                         "digit.3", "equals"}),
                 QStringLiteral("5"));
        // F-E during an error is not taken either
        QCOMPARE(result({"view.scientific", "digit.1", "divide", "digit.0", "equals", "scientific_notation", "clear", "digit.2", "add",
                         "digit.3", "equals"}),
                 QStringLiteral("5"));
    }

    // While an error shows, a key the engine does not take leaves the expression line as it is, in both views
    static void anErrorKeepsTheExpressionLine()
    {
        chrono_calc::Calculator standard(commaSeparators);
        for (const std::string_view command : {"digit.5", "add", "digit.0", "reciprocal", "equals", "digit.7", "memory.recall"}) {
            standard.run(command);
        }
        QVERIFY(standard.isError());
        QCOMPARE(QString::fromStdWString(standard.expression()), QStringLiteral("5 + reciproc(0)"));
        chrono_calc::Calculator scientific(commaSeparators);
        for (const std::string_view command : {"view.scientific", "digit.1", "divide", "digit.0", "equals", "digit.7", "angle.radians", "sin"}) {
            scientific.run(command);
        }
        QVERIFY(scientific.isError());
        QCOMPARE(QString::fromStdWString(scientific.expression()), QStringLiteral("1 /"));
    }

    // Standard: Windows 7 starts with digit grouping off (1234, not 1.234), in both views
    static void numbersAreNotGrouped()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        for (const std::string_view command : {"digit.1", "digit.2", "digit.3", "digit.4"}) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("1234"));
    }
};

QTEST_GUILESS_MAIN(TestReferenceScientific)
#include "test_reference_scientific.moc"
