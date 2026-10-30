#include <QObject>
#include <QString>
#include <QTest>

#include "core/calculator.h"
#include "core/calendar.h"
#include "core/commands.h"
#include "core/datecalculation.h"
#include "core/datecalculator.h"
#include "core/enginetexts.h"
#include "core/view.h"

#include <array>
#include <cstddef>
#include <string>
#include <vector>

// --- Version ------------------------------------------------------------
// tests/test_reference_datecalculation.cpp 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Windows 7 Date calculation results and behaviours of tests/reference/windows7-datecalculation.md
//          (M7)

// The lines of tests/reference/windows7-datecalculation.md, measured in the Windows 7 Calculator. Dates are written as
// it showed them (dd/MM/yyyy); a result date as its weekday and day, which the window writes in the regional long date.
namespace {

constexpr chrono_calc::Separators commaSeparators{.decimal = L',', .thousands = L'.'};

struct DifferenceRow
{
    const char* name;
    const char* from;
    const char* to;
    const char* difference;
    const char* days;
};

struct ShiftRow
{
    const char* name;
    const char* from;
    bool subtract;
    const char* years;
    const char* months;
    const char* days;
    // "Saturday, 28/02/2026", or "Date out of bound"
    const char* result;
};

// Part D line 2
std::vector<DifferenceRow> differenceRows()
{
    return {
        {"D 1", "01/01/2026", "01/01/2026", "Same dates", "Same dates"},
        {"D 2", "01/01/2026", "02/01/2026", "1 day", "1 day"},
        {"D 3", "01/01/2026", "08/01/2026", "1 week", "7 days"},
        {"D 4", "01/01/2026", "15/01/2026", "2 weeks", "14 days"},
        {"D 5", "31/01/2026", "28/02/2026", "1 month", "28 days"},
        {"D 6", "31/01/2026", "01/03/2026", "1 month, 1 day", "29 days"},
        {"D 7", "30/01/2026", "28/02/2026", "1 month", "29 days"},
        {"D 8", "28/01/2026", "28/02/2026", "1 month", "31 days"},
        {"D 9", "29/02/2024", "28/02/2025", "1 year", "365 days"},
        {"D 10", "29/02/2024", "01/03/2025", "1 year, 1 day", "366 days"},
        {"D 11", "29/02/2024", "29/02/2028", "4 years", "1461 days"},
        {"D 12", "31/03/2023", "30/04/2023", "1 month", "30 days"},
        {"D 13", "31/12/2023", "31/12/2024", "1 year", "366 days"},
        {"D 14", "31/05/2026", "30/06/2026", "1 month", "30 days"},
        {"D 15", "15/03/2026", "14/05/2026", "1 month, 4 weeks, 1 day", "60 days"},
        {"D 16", "15/01/2026", "20/03/2027", "1 year, 2 months, 5 days", "429 days"},
        {"D 17", "25/12/2025", "01/01/2026", "1 week", "7 days"},
        {"D 18", "28/02/1900", "01/03/1900", "1 day", "1 day"},
        {"D 19", "01/01/2000", "30/10/2026", "26 years, 9 months, 4 weeks, 1 day", "9799 days"},
        {"D 20", "30/10/2026", "01/01/2000", "26 years, 9 months, 4 weeks, 1 day", "9799 days"},
        {"D 21", "28/03/2026", "30/03/2026", "2 days", "2 days"},
        {"D 22", "01/01/1601", "31/12/9999", "8398 years, 11 months, 4 weeks, 2 days", "3067670 days"},
        {"D 23", "28/02/2026", "31/03/2026", "1 month, 3 days", "31 days"},
        {"D 24", "31/01/2025", "31/03/2025", "2 months", "59 days"},
    };
}

// Part E line 2
std::vector<ShiftRow> shiftRows()
{
    return {
        {"E 1", "31/01/2026", false, "0", "1", "0", "Saturday, 28/02/2026"},
        {"E 2", "31/01/2028", false, "0", "1", "0", "Tuesday, 29/02/2028"},
        {"E 3", "30/01/2026", false, "0", "1", "0", "Saturday, 28/02/2026"},
        {"E 4", "29/02/2024", false, "1", "0", "0", "Friday, 28/02/2025"},
        {"E 5", "29/02/2024", true, "1", "0", "0", "Tuesday, 28/02/2023"},
        {"E 6", "29/02/2024", false, "4", "0", "0", "Tuesday, 29/02/2028"},
        {"E 7", "31/03/2026", true, "0", "1", "0", "Saturday, 28/02/2026"},
        {"E 8", "31/01/2026", false, "0", "1", "1", "Sunday, 01/03/2026"},
        {"E 9", "31/03/2026", true, "0", "1", "1", "Saturday, 28/02/2026"},
        {"E 10", "30/10/2026", false, "0", "0", "0", "Friday, 30/10/2026"},
        {"E 11", "31/12/2026", false, "0", "0", "1", "Friday, 01/01/2027"},
        {"E 12", "01/03/2026", true, "0", "0", "1", "Saturday, 28/02/2026"},
        {"E 13", "30/10/2026", false, "0", "13", "0", "Tuesday, 30/11/2027"},
        {"E 14", "30/10/2026", true, "0", "0", "365", "Thursday, 30/10/2025"},
        {"E 15", "31/08/2026", false, "0", "6", "0", "Sunday, 28/02/2027"},
        {"E 16", "30/10/2026", false, "999", "999", "999", "Thursday, 26/10/3111"},
        {"E 17", "31/12/9999", false, "0", "0", "1", "Date out of bound"},
        {"E 18", "31/12/9999", false, "1", "0", "0", "Date out of bound"},
        {"E 19", "01/01/1601", true, "0", "0", "1", "Date out of bound"},
        {"E 20", "30/10/2026", false, "1", "2", "3", "Sunday, 02/01/2028"},
        {"E 21", "30/10/2026", true, "1", "2", "3", "Wednesday, 27/08/2025"},
        {"E 22", "29/02/2024", false, "1", "1", "0", "Friday, 28/03/2025"},
        {"E 23", "29/02/2024", true, "1", "1", "0", "Sunday, 29/01/2023"},
        {"E 24", "01/03/2026", true, "0", "1", "1", "Wednesday, 28/01/2026"},
        {"E 25", "28/02/2026", false, "0", "1", "1", "Sunday, 29/03/2026"},
        {"E 26", "31/05/2026", true, "0", "1", "30", "Wednesday, 01/04/2026"},
        {"E 27", "31/01/2026", false, "1", "1", "0", "Sunday, 28/02/2027"},
        {"E 28", "30/12/9999", false, "0", "0", "1", "Friday, 31/12/9999"},
        {"E 29", "01/12/9999", false, "0", "1", "0", "Date out of bound"},
        {"E 30", "31/03/2026", true, "0", "0", "0", "Tuesday, 31/03/2026"},
    };
}

// "31/01/2026" as the commands take it: "2026-01-31"
std::wstring iso(const char* shown)
{
    const QString text = QString::fromUtf8(shown);
    return (text.mid(6, 4) + QLatin1Char('-') + text.mid(3, 2) + QLatin1Char('-') + text.mid(0, 2)).toStdWString();
}

QString text(const std::wstring& value)
{
    return QString::fromStdWString(value);
}

// The Add or subtract result as "Saturday, 28/02/2026" (the weekday and the day), or "Date out of bound"
QString shownShift(const chrono_calc::DateCalculator& dates)
{
    static constexpr std::array weekdays{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    const chrono_calc::ShiftedDate& shifted = dates.shifted();
    if (!shifted.shown) {
        return {};
    }
    if (!shifted.date) {
        return text(std::wstring(chrono_calc::outOfBoundText));
    }
    const chrono_calc::Date& date = *shifted.date;
    return QStringLiteral("%1, %2/%3/%4")
        .arg(QString::fromUtf8(weekdays.at(static_cast<std::size_t>(chrono_calc::weekday(date)))))
        .arg(date.day, 2, 10, QLatin1Char('0'))
        .arg(date.month, 2, 10, QLatin1Char('0'))
        .arg(date.year);
}

}  // namespace

class TestReferenceDateCalculation : public QObject
{
    Q_OBJECT

private slots:
    // Part D line 2: every pair, From and To typed, then Calculate
    static void theDifferencesAreWindows7s()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("view.date_calculation"));
        for (const DifferenceRow& row : differenceRows()) {
            QVERIFY2(calculator.run("date.difference_from", iso(row.from)), row.name);
            QVERIFY2(calculator.run("date.difference_to", iso(row.to)), row.name);
            QVERIFY2(calculator.run("date.calculate"), row.name);
            const chrono_calc::DateCalculator& dates = calculator.dateCalculator();
            QVERIFY2(text(dates.differenceText()) == QString::fromUtf8(row.difference), row.name);
            QVERIFY2(text(dates.totalDaysText()) == QString::fromUtf8(row.days), row.name);
        }
    }

    // Part E line 2: the date, Add or Subtract and the three numbers typed, then Calculate
    static void theShiftedDatesAreWindows7s()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        for (const char* command : {"view.date_calculation", "date.mode.1"}) {
            QVERIFY(calculator.run(command));
        }
        for (const ShiftRow& row : shiftRows()) {
            QVERIFY2(calculator.run("date.shift_from", iso(row.from)), row.name);
            QVERIFY2(calculator.run(row.subtract ? "date.subtract" : "date.add"), row.name);
            QVERIFY2(calculator.run("date.years", QString::fromUtf8(row.years).toStdWString()), row.name);
            QVERIFY2(calculator.run("date.months", QString::fromUtf8(row.months).toStdWString()), row.name);
            QVERIFY2(calculator.run("date.days", QString::fromUtf8(row.days).toStdWString()), row.name);
            QVERIFY2(calculator.run("date.calculate"), row.name);
            QVERIFY2(shownShift(calculator.dateCalculator()) == QString::fromUtf8(row.result),
                     qPrintable(QString::fromUtf8(row.name) + QStringLiteral(": ") + shownShift(calculator.dateCalculator())));
        }
    }

    // Part B line 1, part C line 1: the first mode, every date today, the numbers 0, Add, no result
    static void thePanelStartsAtToday()
    {
        const chrono_calc::Date before = chrono_calc::today();
        const chrono_calc::Calculator calculator(commaSeparators);
        const chrono_calc::Date after = chrono_calc::today();
        const chrono_calc::DateCalculator& dates = calculator.dateCalculator();
        QCOMPARE(dates.mode(), chrono_calc::DateMode::Difference);
        // The day may have changed while the calculator was made
        QVERIFY(dates.differenceFrom() == before || dates.differenceFrom() == after);
        QVERIFY(dates.differenceTo() == dates.differenceFrom());
        QVERIFY(dates.shiftFrom() == dates.differenceFrom());
        QVERIFY(!dates.isSubtract());
        for (const chrono_calc::DateField field : {chrono_calc::DateField::Years, chrono_calc::DateField::Months, chrono_calc::DateField::Days}) {
            QCOMPARE(text(dates.field(field)), QStringLiteral("0"));
        }
        QVERIFY(dates.differenceText().empty());
        QVERIFY(dates.totalDaysText().empty());
        QVERIFY(!dates.shifted().shown);
    }

    // Part D line 1: the results only with Calculate; a changed date empties both; the same date again keeps them
    static void aChangedDateEmptiesTheDifference()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("date.difference_from", L"2026-01-01"));
        QVERIFY(calculator.run("date.difference_to", L"2026-01-08"));
        QVERIFY(calculator.dateCalculator().differenceText().empty());
        QVERIFY(calculator.run("date.calculate"));
        QCOMPARE(text(calculator.dateCalculator().differenceText()), QStringLiteral("1 week"));
        QVERIFY(calculator.run("date.difference_to", L"2026-01-08"));
        QCOMPARE(text(calculator.dateCalculator().totalDaysText()), QStringLiteral("7 days"));
        QVERIFY(calculator.run("date.difference_to", L"2026-01-09"));
        QVERIFY(calculator.dateCalculator().differenceText().empty());
        QVERIFY(calculator.dateCalculator().totalDaysText().empty());
    }

    // Part E lines 1 and 4: a changed number, date or Add/Subtract empties the result; Calculate fills it
    static void aChangeEmptiesTheShiftedDate()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("date.mode.1"));
        QVERIFY(calculator.run("date.shift_from", L"2026-10-30"));
        for (const char* change : {"date.subtract", "date.add", "date.days.up", "date.years.down"}) {
            QVERIFY(calculator.run("date.calculate"));
            QVERIFY2(calculator.dateCalculator().shifted().shown, change);
            QVERIFY(calculator.run(change));
            QVERIFY2(!calculator.dateCalculator().shifted().shown, change);
        }
        QVERIFY(calculator.run("date.calculate"));
        QVERIFY(calculator.run("date.months", L"5"));
        QVERIFY(!calculator.dateCalculator().shifted().shown);
        QVERIFY(calculator.run("date.calculate"));
        QVERIFY(calculator.run("date.shift_from", L"2026-10-31"));
        QVERIFY(!calculator.dateCalculator().shifted().shown);
        // Add while Add is chosen changes nothing
        QVERIFY(calculator.run("date.calculate"));
        QVERIFY(calculator.run("date.add"));
        QVERIFY(calculator.dateCalculator().shifted().shown);
    }

    // Part E line 1: digits only, at most three, an empty box is 0 and stays empty; the arrows go round 0 to 999
    static void theNumberBoxesTakeThreeDigits()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("date.mode.1"));
        const chrono_calc::DateCalculator& dates = calculator.dateCalculator();
        const auto years = [&dates] { return text(dates.field(chrono_calc::DateField::Years)); };
        QVERIFY(calculator.run("date.years", L"5a"));
        QCOMPARE(years(), QStringLiteral("5"));
        QVERIFY(calculator.run("date.years", L"99999"));
        QCOMPARE(years(), QStringLiteral("999"));
        QVERIFY(calculator.run("date.years", L"-+x"));
        QCOMPARE(years(), QString());
        QVERIFY(calculator.run("date.shift_from", L"2026-10-30"));
        QVERIFY(calculator.run("date.calculate"));
        QCOMPARE(shownShift(dates), QStringLiteral("Friday, 30/10/2026"));
        QCOMPARE(years(), QString());
        // An empty box: Up is 1, Down is 0; Down at 0 is 999, Up at 999 is 0 (follow-up line 9)
        QVERIFY(calculator.run("date.years.up"));
        QCOMPARE(years(), QStringLiteral("1"));
        QVERIFY(calculator.run("date.years.down"));
        QCOMPARE(years(), QStringLiteral("0"));
        QVERIFY(calculator.run("date.years.down"));
        QCOMPARE(years(), QStringLiteral("999"));
        QVERIFY(calculator.run("date.years.up"));
        QCOMPARE(years(), QStringLiteral("0"));
        QVERIFY(calculator.run("date.days"));
        QVERIFY(calculator.run("date.days.down"));
        QCOMPARE(text(dates.field(chrono_calc::DateField::Days)), QStringLiteral("999"));
        QVERIFY(calculator.run("date.months.up"));
        QCOMPARE(text(dates.field(chrono_calc::DateField::Months)), QStringLiteral("1"));
    }

    // Part C lines 3 and 4: a day that does not exist, or one before 1601 or after 9999, changes nothing
    static void aDateOutsideTheRangeIsRefused()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("date.difference_from", L"2026-04-01"));
        for (const wchar_t* wrong : {L"2026-04-31", L"1600-12-31", L"10000-01-01", L"2025-02-29", L"2026-13-01", L"2026-4-1", L"x"}) {
            QVERIFY(calculator.run("date.difference_from", wrong));
            QCOMPARE(text(chrono_calc::isoText(calculator.dateCalculator().differenceFrom())), QStringLiteral("2026-04-01"));
        }
        for (const wchar_t* right : {L"1601-01-01", L"9999-12-31", L"1752-09-03", L"2024-02-29"}) {
            QVERIFY(calculator.run("date.difference_from", right));
            QCOMPARE(chrono_calc::isoText(calculator.dateCalculator().differenceFrom()), std::wstring(right));
        }
    }

    // Part B line 2, part G line 1: each mode keeps its own over a mode change, a view change, Basic and Unit
    // conversion; the calculator's number is not touched
    static void everythingIsKept()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        for (const char* command : {"digit.7", "view.date_calculation"}) {
            QVERIFY(calculator.run(command));
        }
        QVERIFY(calculator.run("date.difference_from", L"2026-01-01"));
        QVERIFY(calculator.run("date.difference_to", L"2026-01-09"));
        QVERIFY(calculator.run("date.calculate"));
        for (const char* command : {"date.mode.1", "date.subtract"}) {
            QVERIFY(calculator.run(command));
        }
        QVERIFY(calculator.run("date.shift_from", L"2026-10-30"));
        QVERIFY(calculator.run("date.days", L"2"));
        QVERIFY(calculator.run("date.calculate"));
        for (const char* command : {"view.scientific", "view.programmer", "view.statistics", "view.standard", "view.basic",
                                    "view.date_calculation", "view.unit_conversion", "view.date_calculation", "date.mode.0"}) {
            QVERIFY2(calculator.run(command), command);
        }
        const chrono_calc::DateCalculator& dates = calculator.dateCalculator();
        QCOMPARE(text(dates.differenceText()), QStringLiteral("1 week, 1 day"));
        QCOMPARE(text(dates.totalDaysText()), QStringLiteral("8 days"));
        QVERIFY(calculator.run("date.mode.1"));
        QVERIFY(dates.isSubtract());
        QCOMPARE(text(dates.field(chrono_calc::DateField::Days)), QStringLiteral("2"));
        QCOMPARE(shownShift(dates), QStringLiteral("Wednesday, 28/10/2026"));
        QCOMPARE(calculator.sidePanel(), chrono_calc::SidePanel::DateCalculation);
    }

    // The panel's commands never touch the calculator's number or expression
    static void thePanelLeavesTheDisplay()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        for (const char* command : {"digit.2", "add", "digit.3", "view.date_calculation", "date.calculate", "date.mode.1",
                                    "date.days.up", "date.calculate"}) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(text(calculator.display()), QStringLiteral("3"));
        QCOMPARE(text(calculator.expression()), QStringLiteral("2 +"));
    }

    // The ids of the panel
    static void theDateCommandsCarryTheirItem()
    {
        QCOMPARE(chrono_calc::findCommand("date.mode.1").value_or(chrono_calc::CommandTarget{}).kind, chrono_calc::CommandKind::DateMode);
        QCOMPARE(chrono_calc::findCommand("date.mode.1").value_or(chrono_calc::CommandTarget{}).index, std::size_t{1});
        QCOMPARE(chrono_calc::findCommand("date.days.up").value_or(chrono_calc::CommandTarget{}).index, std::size_t{2});
        QCOMPARE(chrono_calc::findCommand("view.date_calculation").value_or(chrono_calc::CommandTarget{}).panel,
                 chrono_calc::SidePanel::DateCalculation);
        for (const char* wrong : {"date.mode.", "date.mode.x", "date.years.left", "date"}) {
            QVERIFY2(!chrono_calc::findCommand(wrong).has_value(), wrong);
        }
        // A mode beyond the list changes nothing
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("date.mode.2"));
        QCOMPARE(calculator.dateCalculator().mode(), chrono_calc::DateMode::Difference);
    }
};

QTEST_GUILESS_MAIN(TestReferenceDateCalculation)
#include "test_reference_datecalculation.moc"
