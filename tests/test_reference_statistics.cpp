#include <QObject>
#include <QString>
#include <QStringList>
#include <QTest>

#include "core/calculator.h"
#include "core/commands.h"
#include "core/enginetexts.h"
#include "core/keys.h"
#include "core/view.h"

#include <cstddef>
#include <initializer_list>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// --- Version ------------------------------------------------------------
// tests/test_reference_statistics.cpp 0.4.11

// --- Version history ----------------------------------------------------
// v0.4.11: Initial: the Windows 7 Statistics view results and behaviours of tests/reference/windows7-statistics.md (M4)

// Each line of parts D and E of tests/reference/windows7-statistics.md, measured in the Windows 7 Calculator: what was
// done in the Statistics view and the number Windows 7 showed. A step is a command id, or characters typed as Windows 7
// reads them in this view ("12,5", "a" for x̄).
namespace {

constexpr chrono_calc::Separators commaSeparators{.decimal = L',', .thousands = L'.'};

constexpr std::string_view enterData = "Enter data to calculate";

struct Line
{
    const char* name;
    const char* steps;
    const char* display;
};

// The values of a list, each followed by Add
std::string listOf(std::initializer_list<const char*> values)
{
    std::string steps;
    for (const char* value : values) {
        steps += std::string(value) + " statistics.add ";
    }
    return steps;
}

std::vector<Line> lines()
{
    return {
        {"E1 5,3,4 x̄", "5 statistics.add 3 statistics.add 4 statistics.add statistics.mean", "4"},
        {"E1 5,3,4 x̄²", "5 statistics.add 3 statistics.add 4 statistics.add statistics.mean_of_squares", "16,66666666666667"},
        {"E1 5,3,4 Σx", "5 statistics.add 3 statistics.add 4 statistics.add statistics.sum", "12"},
        {"E1 5,3,4 Σx²", "5 statistics.add 3 statistics.add 4 statistics.add statistics.sum_of_squares", "50"},
        {"E1 5,3,4 σn", "5 statistics.add 3 statistics.add 4 statistics.add statistics.deviation", "0,816496580927726"},
        {"E1 5,3,4 σn-1", "5 statistics.add 3 statistics.add 4 statistics.add statistics.sample_deviation", "1"},
        {"E2 1-4 x̄", "1 statistics.add 2 statistics.add 3 statistics.add 4 statistics.add statistics.mean", "2,5"},
        {"E2 1-4 x̄²", "1 statistics.add 2 statistics.add 3 statistics.add 4 statistics.add statistics.mean_of_squares", "7,5"},
        {"E2 1-4 Σx", "1 statistics.add 2 statistics.add 3 statistics.add 4 statistics.add statistics.sum", "10"},
        {"E2 1-4 Σx²", "1 statistics.add 2 statistics.add 3 statistics.add 4 statistics.add statistics.sum_of_squares", "30"},
        {"E2 1-4 σn", "1 statistics.add 2 statistics.add 3 statistics.add 4 statistics.add statistics.deviation", "1,118033988749895"},
        {"E2 1-4 σn-1", "1 statistics.add 2 statistics.add 3 statistics.add 4 statistics.add statistics.sample_deviation", "1,290994448735806"},
        {"E3 2,4,4,4,5,5,7,9 x̄", "", "5"},
        {"E3 2,4,4,4,5,5,7,9 x̄²", "", "29"},
        {"E3 2,4,4,4,5,5,7,9 Σx", "", "40"},
        {"E3 2,4,4,4,5,5,7,9 Σx²", "", "232"},
        {"E3 2,4,4,4,5,5,7,9 σn", "", "2"},
        {"E3 2,4,4,4,5,5,7,9 σn-1", "", "2,138089935299395"},
        {"E4 1,2 σn", "1 statistics.add 2 statistics.add statistics.deviation", "0,5"},
        {"E4 1,2 σn-1", "1 statistics.add 2 statistics.add statistics.sample_deviation", "0,7071067811865475"},
        {"E5 1,2,3 σn", "1 statistics.add 2 statistics.add 3 statistics.add statistics.deviation", "0,816496580927726"},
        {"E5 1,2,3 σn-1", "1 statistics.add 2 statistics.add 3 statistics.add statistics.sample_deviation", "1"},
        {"E6 1,1,1 σn", "1 statistics.add 1 statistics.add 1 statistics.add statistics.deviation", "0"},
        {"E7 7 σn", "7 statistics.add statistics.deviation", "0"},
        {"E7 7 σn-1", "7 statistics.add statistics.sample_deviation", "Enter data to calculate"},
        {"E8 -5, 2,5 x̄", "5 negate statistics.add 2,5 statistics.add statistics.mean", "-1,25"},
        {"E9 0,1 0,2 Σx", "0,1 statistics.add 0,2 statistics.add statistics.sum", "0,3"},
        {"E10 9e300 twice Σx", "9 exponent 300 statistics.add 9 exponent 300 statistics.add statistics.sum", "1,8e+301"},
        {"E10 9e300 twice Σx²", "9 exponent 300 statistics.add 9 exponent 300 statistics.add statistics.sum_of_squares", "1,62e+602"},
        {"E11 empty x̄", "statistics.mean", "Enter data to calculate"},
        {"E11 empty x̄²", "statistics.mean_of_squares", "Enter data to calculate"},
        {"E11 empty Σx", "statistics.sum", "Enter data to calculate"},
        {"E11 empty Σx²", "statistics.sum_of_squares", "Enter data to calculate"},
        {"E11 empty σn", "statistics.deviation", "Enter data to calculate"},
        {"E11 empty σn-1", "statistics.sample_deviation", "Enter data to calculate"},
        {"E12 σn added, then x̄²", "5 statistics.add 3 statistics.add 4 statistics.add statistics.deviation statistics.add statistics.mean_of_squares", "12,66666666666667"},
        {"D1 Add shows 0", "5 statistics.add", "0"},
        {"D1 16 digits typed", "12345678901234567", "1234567890123456"},
        {"D2 ± after x̄ changes the number", "5 statistics.add 3 statistics.add 4 statistics.add statistics.mean negate", "-4"},
        {"D3 digits ignored during the error", "statistics.mean 5", "Enter data to calculate"},
        {"D3 C after the error", "statistics.mean clear", "0"},
        {"D6 C keeps the list", "5 statistics.add 3 statistics.add clear statistics.sum", "8"},
        {"D6 MS, MR of Σx, then Add", "5 statistics.add 3 statistics.add 4 statistics.add statistics.sum memory.store clear memory.recall statistics.add statistics.sum", "24"},
        {"C keys: a s t", "5 statistics.add 3 statistics.add 4 statistics.add t", "0,816496580927726"},
    };
}

// The steps of a line: the list 2,4,4,4,5,5,7,9 for E3, each followed by its statistic
std::string stepsOf(const Line& line)
{
    if (std::string_view(line.steps).empty()) {
        const std::string name(line.name);
        std::string statistic = "statistics.sample_deviation";
        if (name.ends_with("x̄")) {
            statistic = "statistics.mean";
        } else if (name.ends_with("x̄²")) {
            statistic = "statistics.mean_of_squares";
        } else if (name.ends_with("Σx")) {
            statistic = "statistics.sum";
        } else if (name.ends_with("Σx²")) {
            statistic = "statistics.sum_of_squares";
        } else if (name.ends_with("σn")) {
            statistic = "statistics.deviation";
        }
        return listOf({"2", "4", "4", "4", "5", "5", "7", "9"}) + statistic;
    }
    return line.steps;
}

// Runs the steps in a calculator that is in the Statistics view
bool runSteps(chrono_calc::Calculator& calculator, const std::string& steps)
{
    std::istringstream words(steps);
    std::string word;
    while (words >> word) {
        if (chrono_calc::findCommand(word)) {
            if (!calculator.run(word)) {
                return false;
            }
            continue;
        }
        for (const char character : word) {
            const std::optional<std::string_view> command =
                chrono_calc::commandForCharacter(static_cast<unsigned char>(character), chrono_calc::View::Statistics);
            if (!command || !calculator.run(*command)) {
                return false;
            }
        }
    }
    return true;
}

QStringList valuesOf(chrono_calc::Calculator& calculator)
{
    QStringList values;
    for (const std::wstring& value : calculator.statisticsValues()) {
        values.append(QString::fromStdWString(value));
    }
    return values;
}

}  // namespace

class TestReferenceStatistics : public QObject
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
        QVERIFY(calculator.run("view.statistics"));
        QVERIFY(runSteps(calculator, stepsOf(line)));
        QCOMPARE(QString::fromStdWString(calculator.display()), QString::fromUtf8(line.display));
        QCOMPARE(calculator.isError(), std::string_view(line.display) == enterData);
        QCOMPARE(QString::fromStdWString(calculator.expression()), QString());
    }

    // Part D 1 and 2: Add puts the number at the end and selects it; a statistic changes nothing in the list; Add right
    // after Add adds 0; the items are written as the display writes numbers
    static void addPutsTheNumberAtTheEndAndSelectsIt()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("view.statistics"));
        QCOMPARE(calculator.statisticsCount(), std::size_t{0});
        QVERIFY(!calculator.selectedValue().has_value());
        QVERIFY(runSteps(calculator, "5, statistics.add 0,10 statistics.add 9 exponent 300 statistics.add statistics.add"));
        QCOMPARE(valuesOf(calculator), QStringList({QStringLiteral("5"), QStringLiteral("0,1"), QStringLiteral("9,e+300"), QStringLiteral("0")}));
        QCOMPARE(calculator.selectedValue(), std::optional<std::size_t>(3));
        QVERIFY(runSteps(calculator, "statistics.sum"));
        QCOMPARE(calculator.statisticsCount(), std::size_t{4});
    }

    // Part D 4 and 5: the selection moves without wrapping; Delete removes the selected value and selects the one that
    // moved up into its place; the last value goes too (Windows 7's fault of keeping it is not copied), then the new
    // last is selected, and an empty list has no selection
    static void deleteRemovesTheSelectedValue()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("view.statistics"));
        QVERIFY(runSteps(calculator, listOf({"5", "3", "4", "6"}) + "7"));
        QVERIFY(calculator.run("statistics.select.1"));
        QVERIFY(calculator.run("statistics.delete"));
        QCOMPARE(valuesOf(calculator), QStringList({QStringLiteral("5"), QStringLiteral("4"), QStringLiteral("6")}));
        QCOMPARE(calculator.selectedValue(), std::optional<std::size_t>(1));
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("7"));
        QVERIFY(calculator.run("statistics.select_previous"));
        QVERIFY(calculator.run("statistics.select_previous"));
        QCOMPARE(calculator.selectedValue(), std::optional<std::size_t>(0));
        QVERIFY(calculator.run("statistics.select.2"));
        QVERIFY(calculator.run("statistics.select_next"));
        QCOMPARE(calculator.selectedValue(), std::optional<std::size_t>(2));
        QVERIFY(calculator.run("statistics.select.9"));
        QCOMPARE(calculator.selectedValue(), std::optional<std::size_t>(2));
        QVERIFY(calculator.run("statistics.delete"));
        QCOMPARE(valuesOf(calculator), QStringList({QStringLiteral("5"), QStringLiteral("4")}));
        QCOMPARE(calculator.selectedValue(), std::optional<std::size_t>(1));
        QVERIFY(calculator.run("statistics.delete"));
        QVERIFY(calculator.run("statistics.delete"));
        QCOMPARE(calculator.statisticsCount(), std::size_t{0});
        QVERIFY(!calculator.selectedValue().has_value());
        QVERIFY(calculator.run("statistics.delete"));
        QVERIFY(calculator.run("statistics.select_next"));
        QVERIFY(!calculator.selectedValue().has_value());
    }

    // Part D 3 and 6: during "Enter data to calculate" only C and CAD work; CAD clears the list, C keeps it; the memory
    // stays with both
    static void onlyCAndCadEndTheError()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("view.statistics"));
        QVERIFY(runSteps(calculator, "8 memory.store 7 statistics.add statistics.sample_deviation"));
        QVERIFY(calculator.isError());
        QVERIFY(runSteps(calculator, "5 statistics.add statistics.mean negate memory.store memory.recall backspace scientific_notation exponent"));
        QCOMPARE(QString::fromStdWString(calculator.display()), QString::fromUtf8(enterData.data()));
        QCOMPARE(calculator.statisticsCount(), std::size_t{1});
        QVERIFY(!calculator.isScientificNotation());
        QVERIFY(calculator.run("clear"));
        QVERIFY(!calculator.isError());
        QCOMPARE(calculator.statisticsCount(), std::size_t{1});
        QVERIFY(runSteps(calculator, "statistics.sample_deviation statistics.clear_all"));
        QVERIFY(!calculator.isError());
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("0"));
        QCOMPARE(calculator.statisticsCount(), std::size_t{0});
        QVERIFY(calculator.hasMemory());
        QVERIFY(calculator.run("memory.recall"));
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("8"));
    }

    // Part D 7: a view change clears the number, keeps the list and the memory, and turns F-E off; F-E writes the items
    // in its notation and survives C and CAD
    static void aViewChangeKeepsTheListAndEndsFe()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("view.statistics"));
        QVERIFY(runSteps(calculator, listOf({"5", "3", "4"}) + "7 memory.store 8 scientific_notation"));
        QVERIFY(calculator.isScientificNotation());
        QCOMPARE(valuesOf(calculator).front(), QStringLiteral("5,e+0"));
        QVERIFY(calculator.run("clear"));
        QVERIFY(calculator.isScientificNotation());
        QVERIFY(calculator.run("view.standard"));
        QVERIFY(!calculator.isScientificNotation());
        QVERIFY(runSteps(calculator, "statistics.add"));
        QVERIFY(calculator.run("digit.7"));
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("7"));
        QVERIFY(calculator.run("view.statistics"));
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("0"));
        QCOMPARE(valuesOf(calculator), QStringList({QStringLiteral("5"), QStringLiteral("3"), QStringLiteral("4")}));
        QVERIFY(calculator.hasMemory());
        QVERIFY(!calculator.isScientificNotation());
        QVERIFY(calculator.run("view.scientific"));
        QVERIFY(calculator.run("digit.7"));
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("7"));
        QVERIFY(calculator.run("view.statistics"));
        QVERIFY(runSteps(calculator, "scientific_notation statistics.clear_all"));
        QVERIFY(calculator.isScientificNotation());
    }

    // Part C: the keys of the Statistics view; the operators and the other letters do nothing there
    static void theStatisticsKeysGiveTheirWindows7Commands()
    {
        using chrono_calc::Key;
        constexpr auto statistics = chrono_calc::View::Statistics;
        for (const auto& [character, command] : {std::pair{U'a', "statistics.mean"}, std::pair{U's', "statistics.sum"},
                                                 std::pair{U't', "statistics.deviation"}, std::pair{U'd', "statistics.clear_all"},
                                                 std::pair{U'x', "exponent"}, std::pair{U'e', "exponent"},
                                                 std::pair{U'v', "scientific_notation"}, std::pair{U',', "decimal"},
                                                 std::pair{U'\r', "statistics.add"}, std::pair{U'9', "digit.9"}}) {
            QCOMPARE(chrono_calc::commandForCharacter(character, statistics).value_or(""), std::string_view(command));
        }
        for (const char32_t character : U"+-*/=@#%;!()qrnlmpoyicgbuwzfhjkASTD") {
            if (character != U'\0') {
                QVERIFY(!chrono_calc::commandForCharacter(character, statistics).has_value());
            }
        }
        QCOMPARE(chrono_calc::commandForKey(Key::Enter, statistics).value_or(""), "statistics.add");
        QCOMPARE(chrono_calc::commandForKey(Key::Escape, statistics).value_or(""), "clear");
        QCOMPARE(chrono_calc::commandForKey(Key::Backspace, statistics).value_or(""), "backspace");
        QCOMPARE(chrono_calc::commandForKey(Key::F9, statistics).value_or(""), "negate");
        QVERIFY(!chrono_calc::commandForKey(Key::Delete, statistics).has_value());
        QCOMPARE(chrono_calc::commandForControlLetter(U'A', statistics).value_or(""), "statistics.mean_of_squares");
        QCOMPARE(chrono_calc::commandForControlLetter(U'S', statistics).value_or(""), "statistics.sum_of_squares");
        QCOMPARE(chrono_calc::commandForControlLetter(U'T', statistics).value_or(""), "statistics.sample_deviation");
        QCOMPARE(chrono_calc::commandForControlLetter(U'M', statistics).value_or(""), "memory.store");
        QVERIFY(!chrono_calc::commandForControlLetter(U'D', statistics).has_value());
        QVERIFY(!chrono_calc::commandForControlLetter(U'A', chrono_calc::View::Standard).has_value());
        QVERIFY(!chrono_calc::commandForCharacter(U'a', chrono_calc::View::Standard).has_value());
    }

    // "statistics.select.N" carries its index in the id: only digits, at most nine of them
    static void theSelectCommandReadsItsIndex()
    {
        QCOMPARE(chrono_calc::findCommand("statistics.select.12").value_or(chrono_calc::CommandTarget{}).index, std::size_t{12});
        for (const char* wrong : {"statistics.select.", "statistics.select.x", "statistics.select.-1", "statistics.select.1234567890",
                                  "statistics.select"}) {
            QVERIFY2(!chrono_calc::findCommand(wrong).has_value(), wrong);
        }
    }

    // The statistics commands do nothing outside the Statistics view
    static void theStatisticsCommandsDoNothingElsewhere()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(runSteps(calculator, "5 statistics.add statistics.mean statistics.clear_all"));
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("5"));
        QCOMPARE(calculator.statisticsCount(), std::size_t{0});
    }
};

QTEST_GUILESS_MAIN(TestReferenceStatistics)
#include "test_reference_statistics.moc"
