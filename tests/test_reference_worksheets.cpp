#include <QObject>
#include <QString>
#include <QTest>

#include "core/calculator.h"
#include "core/enginetexts.h"
#include "core/worksheetcalculator.h"
#include "core/worksheets.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// --- Version ------------------------------------------------------------
// tests/test_reference_worksheets.cpp 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Windows 7 Worksheets results and behaviours of tests/reference/windows7-worksheets.md (M8)

// The lines of tests/reference/windows7-worksheets.md, measured in the Windows 7 Calculator with the regional decimal
// symbol ','. Where Chrono Calc differs (an interest rate of 0, D-027), the test holds Chrono Calc's value and the
// reference says Windows 7's.
namespace {

constexpr chrono_calc::Separators commaSeparators{.decimal = L',', .thousands = L'.'};

// One line of the reference: the worksheet (its View command), the value calculated, the boxes as typed in their order,
// and the result: its text, "UC" (Unable to calculate) or "NV <value>" (This value is not valid. on that value's box)
struct Line
{
    const char* name;
    const char* panel;
    const char* calculate;
    std::vector<const char*> boxes;
    const char* result;
};

constexpr const char* mortgage = "view.mortgage";
constexpr const char* lease = "view.vehicle_lease";
constexpr const char* mpg = "view.fuel_economy_mpg";
constexpr const char* litres = "view.fuel_economy_litres";

// Part C1
std::vector<Line> mortgageLines()
{
    const char* payment = "Monthly payment";
    const char* down = "Down payment";
    const char* price = "Purchase price";
    const char* term = "Term (years)";
    return {
        {"C1 1", mortgage, payment, {"250000", "50000", "30", "4,5"}, "1013,370619651761"},
        {"C1 2 Chrono Calc", mortgage, payment, {"250000", "50000", "30", "0"}, "555,5555555555556"},
        {"C1 3", mortgage, payment, {"250000", "50000", "30", "0,001"}, "555,6391245370031"},
        {"C1 4", mortgage, payment, {"250000", "50000", "1", "4,5"}, "17075,70431727441"},
        {"C1 5", mortgage, payment, {"250000", "50000", "0,5", "4,5"}, "33772,19795195795"},
        {"C1 6", mortgage, payment, {"250000", "50000", "30", "100"}, "16666,66666667177"},
        {"C1 7", mortgage, payment, {"1000000000", "0", "30", "4,5"}, "5066853,098258807"},
        {"C1 8", mortgage, payment, {"100", "0", "1", "12"}, "8,884878867834171"},
        {"C1 9", mortgage, payment, {"250000", "300000", "30", "4,5"}, "-253,3426549129403"},
        {"C1 10", mortgage, payment, {"250000", "50000", "0", "4,5"}, "UC"},
        {"C1 11", mortgage, payment, {"250000", "50000", "30", "-5"}, "NV Interest rate (%)"},
        {"C1 12", mortgage, payment, {"250000", "50000", "1000", "10000"}, "1666666,666666667"},
        {"C1 13", mortgage, payment, {"250000", "50000", "1000", "1e4"}, "1,666666666666667e+6"},
        {"C1 14", mortgage, payment, {"250000", "50000", "1000", "4,5"}, "750"},
        {"C1 15", mortgage, down, {"250000", "30", "4,5", "1013,370619651761"}, "50000,00000000008"},
        {"C1 16", mortgage, down, {"250000", "30", "4,5", "1000"}, "52638,84099113829"},
        {"C1 17", mortgage, down, {"250000", "30", "4,5", "2000"}, "-144722,3180177234"},
        {"C1 18", mortgage, down, {"250000", "10", "3", "1500"}, "94657,37037495967"},
        {"C1 19", mortgage, down, {"300000", "30", "0,001", "500"}, "120027,0722776642"},
        {"C1 20 Chrono Calc", mortgage, down, {"250000", "30", "0", "1000"}, "-110000"},
        {"C1 21", mortgage, down, {"250000", "0,5", "4,5", "33772,19795195795"}, "50000,00000000002"},
        {"C1 22", mortgage, down, {"100", "1", "12", "1"}, "88,74492252651537"},
        {"C1 23", mortgage, price, {"50000", "30", "4,5", "1013,370619651761"}, "249999,9999999999"},
        {"C1 24", mortgage, price, {"0", "30", "4,5", "1000"}, "197361,1590088617"},
        {"C1 25", mortgage, price, {"50000", "15", "6", "2000"}, "287007,0293352141"},
        {"C1 26 Chrono Calc", mortgage, price, {"0", "30", "0", "1000"}, "360000"},
        {"C1 27", mortgage, price, {"0", "1", "12", "8,884878867834171"}, "100"},
        {"C1 28", mortgage, price, {"0", "0,5", "4,5", "100"}, "592,2030904962315"},
        {"C1 29", mortgage, price, {"0", "30", "100", "1000"}, "11999,99999999633"},
        {"C1 30", mortgage, price, {"-1000", "30", "4,5", "1000"}, "NV Down payment"},
        {"C1 31", mortgage, term, {"250000", "50000", "4,5", "1013,370619651761"}, "30,00000000000002"},
        {"C1 32", mortgage, term, {"250000", "50000", "4,5", "1500"}, "15,43213379469031"},
        {"C1 33", mortgage, term, {"250000", "50000", "4,5", "750"}, "UC"},
        {"C1 34", mortgage, term, {"250000", "50000", "4,5", "500"}, "UC"},
        {"C1 35 Chrono Calc", mortgage, term, {"250000", "50000", "0", "1000"}, "16,66666666666667"},
        {"C1 36", mortgage, term, {"100", "0", "12", "8,884878867834171"}, "1"},
        {"C1 37", mortgage, term, {"250000", "50000", "0,001", "1000"}, "16,66806265491808"},
        {"C1 38", mortgage, term, {"250000", "0", "6", "1250"}, "UC"},
        {"C1 39", mortgage, term, {"100000", "150000", "5", "500"}, "-6,980636606914711"},
        {"C1 40", mortgage, term, {"250000", "50000", "4,5", "0"}, "UC"},
    };
}

// Part C2
std::vector<Line> leaseLines()
{
    const char* payment = "Periodic payment";
    const char* value = "Lease value";
    const char* residual = "Residual value";
    const char* period = "Lease period";
    return {
        {"C2 1", lease, payment, {"30000", "3", "12", "10000", "5"}, "641,084608759976"},
        {"C2 2 Chrono Calc", lease, payment, {"30000", "3", "12", "10000", "0"}, "555,5555555555556"},
        {"C2 3", lease, payment, {"30000", "3", "12", "0", "5"}, "899,1269131399641"},
        {"C2 4", lease, payment, {"30000", "3", "12", "30000", "5"}, "125"},
        {"C2 5", lease, payment, {"30000", "3", "1", "10000", "5"}, "7844,171292624901"},
        {"C2 6", lease, payment, {"30000", "3", "4", "10000", "5"}, "1930,166246903139"},
        {"C2 7", lease, payment, {"30000", "3", "52", "10000", "5"}, "147,7377037974495"},
        {"C2 8", lease, payment, {"30000", "3", "7", "10000", "5"}, "1100,414125707024"},
        {"C2 9", lease, payment, {"30000", "3", "0", "10000", "5"}, "UC"},
        {"C2 10", lease, payment, {"30000", "3", "12", "40000", "5"}, "-133,042304379988"},
        {"C2 11", lease, payment, {"30000", "3", "7,5", "10000", "5"}, "1026,842421142456"},
        {"C2 12", lease, payment, {"30000", "0,1", "12", "10000", "5"}, "16784,73280974062"},
        {"C2 13", lease, value, {"3", "12", "10000", "5", "641,084608759976"}, "30000"},
        {"C2 14", lease, value, {"3", "12", "0", "5", "500"}, "16682,8506418704"},
        {"C2 15 Chrono Calc", lease, value, {"3", "12", "10000", "0", "500"}, "28000"},
        {"C2 16", lease, value, {"3", "1", "10000", "5", "7844,171292624901"}, "30000"},
        {"C2 17", lease, value, {"5", "12", "5000", "3,5", "300"}, "20689,35069908491"},
        {"C2 18", lease, value, {"3", "7", "10000", "5", "1100,414125707024"}, "30000"},
        {"C2 19", lease, value, {"0,5", "12", "0", "5", "1000"}, "5913,463183017656"},
        {"C2 20", lease, value, {"3", "12", "10000", "5", "0"}, "8609,7624465108"},
        {"C2 21", lease, residual, {"30000", "3", "12", "5", "641,084608759976"}, "10000"},
        {"C2 22", lease, residual, {"30000", "3", "12", "5", "0"}, "34844,16694000405"},
        {"C2 23", lease, residual, {"30000", "3", "12", "5", "1000"}, "-3909,168580028343"},
        {"C2 24 Chrono Calc", lease, residual, {"30000", "3", "12", "0", "500"}, "12000"},
        {"C2 25", lease, residual, {"30000", "3", "1", "5", "7844,171292624901"}, "10000"},
        {"C2 26", lease, residual, {"20000", "4", "4", "6", "1200"}, "3860,867140748894"},
        {"C2 27", lease, residual, {"30000", "3", "12", "5", "125"}, "30000"},
        {"C2 28", lease, residual, {"30000", "3", "52", "5", "147,7377037974495"}, "9999,999999999994"},
        {"C2 29", lease, period, {"30000", "12", "10000", "5", "641,084608759976"}, "3"},
        {"C2 30", lease, period, {"30000", "12", "10000", "5", "800"}, "2,333054106858931"},
        {"C2 31", lease, period, {"30000", "12", "10000", "5", "100"}, "UC"},
        {"C2 32 Chrono Calc", lease, period, {"30000", "12", "10000", "0", "500"}, "3,333333333333333"},
        {"C2 33", lease, period, {"30000", "1", "10000", "5", "7844,171292624901"}, "3"},
        {"C2 34", lease, period, {"30000", "12", "0", "5", "1000"}, "2,676187804789212"},
        {"C2 35", lease, period, {"30000", "12", "30000", "5", "125"}, "UC"},
        {"C2 36", lease, period, {"30000", "4", "10000", "5", "1500"}, "4,038445812231709"},
        {"C2 37", lease, period, {"30000", "12", "40000", "5", "100"}, "19,65742462427103"},
    };
}

// Part C3
std::vector<Line> fuelLines()
{
    const char* mpgEconomy = "Fuel economy (mpg)";
    const char* miles = "Distance (miles)";
    const char* gallons = "Fuel used (gallons)";
    const char* litresEconomy = "Fuel economy (L/100 km)";
    const char* kilometres = "Distance (kilometers)";
    const char* used = "Fuel used (liters)";
    return {
        {"C3 1", mpg, mpgEconomy, {"100", "3"}, "33,33333333333333"},
        {"C3 2", mpg, mpgEconomy, {"300", "10"}, "30"},
        {"C3 3", mpg, mpgEconomy, {"1", "3"}, "0,3333333333333333"},
        {"C3 4", mpg, mpgEconomy, {"0", "5"}, "0"},
        {"C3 5", mpg, mpgEconomy, {"100", "0"}, "UC"},
        {"C3 6", mpg, mpgEconomy, {"9999999999999999", "1"}, "9999999999999999"},
        {"C3 7", mpg, mpgEconomy, {"2", "3"}, "0,6666666666666667"},
        {"C3 8", mpg, miles, {"10", "30"}, "300"},
        {"C3 9", mpg, miles, {"3", "33,33333333333333"}, "99,99999999999999"},
        {"C3 10", mpg, miles, {"0", "30"}, "0"},
        {"C3 11", mpg, miles, {"10", "0"}, "0"},
        {"C3 12", mpg, miles, {"1,5", "2,5"}, "3,75"},
        {"C3 13", mpg, miles, {"7", "3"}, "21"},
        {"C3 14", mpg, gallons, {"300", "30"}, "10"},
        {"C3 15", mpg, gallons, {"100", "3"}, "33,33333333333333"},
        {"C3 16", mpg, gallons, {"0", "30"}, "0"},
        {"C3 17", mpg, gallons, {"100", "0"}, "UC"},
        {"C3 18", mpg, gallons, {"1", "7"}, "0,1428571428571429"},
        {"C3 19", mpg, gallons, {"2,5", "1,5"}, "1,666666666666667"},
        {"C3 20", litres, litresEconomy, {"100", "3"}, "3"},
        {"C3 21", litres, litresEconomy, {"300", "10"}, "3,333333333333333"},
        {"C3 22", litres, litresEconomy, {"3", "1"}, "33,33333333333333"},
        {"C3 23", litres, litresEconomy, {"0", "5"}, "UC"},
        {"C3 24", litres, litresEconomy, {"100", "0"}, "0"},
        {"C3 25", litres, litresEconomy, {"7", "1"}, "14,28571428571429"},
        {"C3 26", litres, litresEconomy, {"9999999999999999", "1"}, "0,00000000000001"},
        {"C3 27", litres, kilometres, {"10", "5"}, "200"},
        {"C3 28", litres, kilometres, {"3", "3,333333333333333"}, "90,00000000000001"},
        {"C3 29", litres, kilometres, {"0", "5"}, "0"},
        {"C3 30", litres, kilometres, {"10", "0"}, "UC"},
        {"C3 31", litres, kilometres, {"1", "7"}, "14,28571428571429"},
        {"C3 32", litres, kilometres, {"2,5", "1,5"}, "166,6666666666667"},
        {"C3 33", litres, used, {"200", "5"}, "10"},
        {"C3 34", litres, used, {"100", "3"}, "3"},
        {"C3 35", litres, used, {"0", "5"}, "0"},
        {"C3 36", litres, used, {"100", "0"}, "0"},
        {"C3 37", litres, used, {"7", "3"}, "0,21"},
        {"C3 38", litres, used, {"1", "1"}, "0,01"},
    };
}

// Part E: Fuel economy (mpg) with the Distance given and Fuel used 1
std::vector<Line> numberLines()
{
    const char* economy = "Fuel economy (mpg)";
    return {
        {"E1 1", mpg, economy, {"1e5", "1"}, "1,e+5"},
        {"E1 2", mpg, economy, {"1,5e3", "1"}, "1,5e+3"},
        {"E1 3", mpg, economy, {"1e-5", "1"}, "1,e-5"},
        {"E1 4", mpg, economy, {"1e400", "1"}, "1,e+400"},
        {"E1 5", mpg, economy, {"1e+5", "1"}, "1,e+5"},
        {"E1 6", mpg, economy, {"12345678901234567", "1"}, "NV Distance (miles)"},
        {"E1 7", mpg, economy, {"1234567890123456", "1"}, "1234567890123456"},
        {"E1 8", mpg, economy, {"0,12345678901234567", "1"}, "NV Distance (miles)"},
        {"E1 9", mpg, economy, {"00000000000000001", "1"}, "1"},
        {"E1 10", mpg, economy, {"+5", "1"}, "5"},
        {"E1 11", mpg, economy, {"--5", "1"}, "NV Distance (miles)"},
        {"E1 12", mpg, economy, {"1,5,5", "1"}, "NV Distance (miles)"},
        {"E1 13", mpg, economy, {"5-", "1"}, "NV Distance (miles)"},
        {"E1 14", mpg, economy, {"e5", "1"}, "1,e+5"},
        {"E1 15", mpg, economy, {"1e", "1"}, "1,e+0"},
        {"E1 16", mpg, economy, {"-0", "1"}, "NV Distance (miles)"},
        {"E1 17", mpg, economy, {"0,0000000000000000001", "1"}, "NV Distance (miles)"},
        {"E1 18", mpg, economy, {"100000", "1"}, "100000"},
        {"E2 1", mpg, economy, {"9999999999999999", "0,0001"}, "9,999999999999999e+19"},
        {"E2 2", mpg, economy, {"1", "9999999999999999"}, "0,0000000000000001"},
        {"E2 3", mpg, economy, {"1e300", "1e-300"}, "1,e+600"},
        {"E2 4", mpg, economy, {"1e300", "1e-5"}, "1,e+305"},
        {"E2 5", mpg, economy, {"1", "3e-20"}, "3,333333333333333e+19"},
    };
}

QString text(std::wstring_view value)
{
    return QString::fromStdWString(std::wstring(value));
}

chrono_calc::Worksheet worksheetShown(const chrono_calc::Calculator& calculator)
{
    return chrono_calc::worksheetOf(calculator.sidePanel()).value_or(chrono_calc::Worksheet::Mortgage);
}

// The value's place in the worksheet's own order
std::optional<std::size_t> valueOf(chrono_calc::Worksheet worksheet, std::string_view name)
{
    const std::vector<std::string_view>& values = chrono_calc::worksheetSpec(worksheet).values;
    const auto found = std::ranges::find(values, name);
    return found == values.end() ? std::nullopt : std::optional(static_cast<std::size_t>(found - values.begin()));
}

// Chooses the value to calculate from the list, as the user does
bool choose(chrono_calc::Calculator& calculator, std::string_view name)
{
    const chrono_calc::Worksheet worksheet = worksheetShown(calculator);
    const std::optional<std::size_t> value = valueOf(worksheet, name);
    const std::vector<std::size_t>& choices = chrono_calc::worksheetSpec(worksheet).choices;
    const auto item = std::ranges::find(choices, value.value_or(choices.size()));
    return item != choices.end() && calculator.run("worksheet.choose." + std::to_string(item - choices.begin()));
}

// Types the texts into the boxes in their order
bool type(chrono_calc::Calculator& calculator, const std::vector<const char*>& boxes)
{
    const chrono_calc::Worksheet worksheet = worksheetShown(calculator);
    const std::vector<std::size_t> values = calculator.worksheetCalculator().boxValues(worksheet);
    if (values.size() != boxes.size()) {
        return false;
    }
    for (std::size_t index = 0; index < boxes.size(); ++index) {
        if (!calculator.run("worksheet.value." + std::to_string(values.at(index)), QString::fromUtf8(boxes.at(index)).toStdWString())) {
            return false;
        }
    }
    return true;
}

// The result as the reference writes it: the result box, "UC" or "NV <value>"
QString outcome(const chrono_calc::Calculator& calculator)
{
    const chrono_calc::Worksheet worksheet = worksheetShown(calculator);
    const chrono_calc::WorksheetState& state = calculator.worksheetCalculator().state(worksheet);
    switch (state.message) {
    case chrono_calc::WorksheetMessage::UnableToCalculate:
        return QStringLiteral("UC");
    case chrono_calc::WorksheetMessage::NotValid:
        return QStringLiteral("NV ") + QString::fromUtf8(chrono_calc::worksheetSpec(worksheet).values.at(state.messageValue));
    case chrono_calc::WorksheetMessage::Blank:
        return QStringLiteral("Blank ") + QString::fromUtf8(chrono_calc::worksheetSpec(worksheet).values.at(state.messageValue));
    case chrono_calc::WorksheetMessage::None:
        break;
    }
    return text(state.texts.at(state.chosen));
}

QString result(const chrono_calc::Calculator& calculator)
{
    const chrono_calc::WorksheetState& state = calculator.worksheetCalculator().state(worksheetShown(calculator));
    return text(state.texts.at(state.chosen));
}

QString box(const chrono_calc::Calculator& calculator, std::string_view name)
{
    const chrono_calc::Worksheet worksheet = worksheetShown(calculator);
    return text(calculator.worksheetCalculator().state(worksheet).texts.at(valueOf(worksheet, name).value_or(0)));
}

void addRows(const std::vector<Line>& lines)
{
    QTest::addColumn<int>("line");
    for (std::size_t line = 0; line < lines.size(); ++line) {
        QTest::newRow(lines.at(line).name) << static_cast<int>(line);
    }
}

QString calculated(const Line& line)
{
    chrono_calc::Calculator calculator(commaSeparators);
    if (!calculator.run(line.panel) || !choose(calculator, line.calculate) || !type(calculator, line.boxes)
        || !calculator.run("worksheet.calculate")) {
        return QStringLiteral("cannot run");
    }
    return outcome(calculator);
}

// A calculator showing Mortgage with the first line of part C1 calculated
void calculateFirstMortgage(chrono_calc::Calculator& calculator)
{
    QVERIFY(calculator.run("view.mortgage"));
    QVERIFY(type(calculator, {"250000", "50000", "30", "4,5"}));
    QVERIFY(calculator.run("worksheet.calculate"));
    QCOMPARE(result(calculator), QStringLiteral("1013,370619651761"));
}

}  // namespace

class TestReferenceWorksheets : public QObject
{
    Q_OBJECT

private slots:
    // Part A line 3, part B line 1: the values, the lists and the value chosen the first time
    static void theWorksheetsAreWindows7s()
    {
        using chrono_calc::Worksheet;
        const std::vector<std::vector<std::string_view>> lists = {
            {"Down payment", "Monthly payment", "Purchase price", "Term (years)"},
            {"Lease period", "Lease value", "Periodic payment", "Residual value"},
            {"Distance (miles)", "Fuel economy (mpg)", "Fuel used (gallons)"},
            {"Distance (kilometers)", "Fuel economy (L/100 km)", "Fuel used (liters)"},
        };
        const std::vector<std::string_view> first = {"Monthly payment", "Periodic payment", "Fuel economy (mpg)", "Fuel economy (L/100 km)"};
        const std::vector<std::string_view> names = {"Mortgage", "Vehicle lease", "Fuel economy (mpg)", "Fuel economy (L/100 km)"};
        for (std::size_t index = 0; index < chrono_calc::worksheetCount; ++index) {
            const chrono_calc::WorksheetSpec& spec = chrono_calc::worksheets().at(index);
            QCOMPARE(spec.name, names.at(index));
            QCOMPARE(spec.choices.size(), lists.at(index).size());
            for (std::size_t item = 0; item < spec.choices.size(); ++item) {
                QCOMPARE(spec.values.at(spec.choices.at(item)), lists.at(index).at(item));
            }
            QCOMPARE(spec.values.at(spec.firstChoice), first.at(index));
        }
        QCOMPARE(chrono_calc::worksheetSpec(Worksheet::Mortgage).values,
                 (std::vector<std::string_view>{"Purchase price", "Down payment", "Term (years)", "Interest rate (%)", "Monthly payment"}));
        QCOMPARE(chrono_calc::worksheetSpec(Worksheet::VehicleLease).values,
                 (std::vector<std::string_view>{"Lease value", "Lease period", "Payments per year", "Residual value", "Interest rate (%)",
                                                "Periodic payment"}));
    }

    static void mortgageResults_data()
    {
        addRows(mortgageLines());
    }

    static void mortgageResults()
    {
        QFETCH(int, line);  // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        const Line reference = mortgageLines().at(static_cast<std::size_t>(line));
        QCOMPARE(calculated(reference), QString::fromUtf8(reference.result));
    }

    static void leaseResults_data()
    {
        addRows(leaseLines());
    }

    static void leaseResults()
    {
        QFETCH(int, line);  // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        const Line reference = leaseLines().at(static_cast<std::size_t>(line));
        QCOMPARE(calculated(reference), QString::fromUtf8(reference.result));
    }

    static void fuelResults_data()
    {
        addRows(fuelLines());
    }

    static void fuelResults()
    {
        QFETCH(int, line);  // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        const Line reference = fuelLines().at(static_cast<std::size_t>(line));
        QCOMPARE(calculated(reference), QString::fromUtf8(reference.result));
    }

    // Part E
    static void numbersAsWindows7ReadsAndWritesThem_data()
    {
        addRows(numberLines());
    }

    static void numbersAsWindows7ReadsAndWritesThem()
    {
        QFETCH(int, line);  // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        const Line reference = numberLines().at(static_cast<std::size_t>(line));
        QCOMPARE(calculated(reference), QString::fromUtf8(reference.result));
    }

    // Part B line 1: choosing another value moves the result into a box and the chosen value's text into the result
    static void choosingAnotherValueMovesTheResult()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        calculateFirstMortgage(calculator);
        QVERIFY(choose(calculator, "Term (years)"));
        QCOMPARE(result(calculator), QStringLiteral("30"));
        const std::vector<std::size_t> boxes = calculator.worksheetCalculator().boxValues(chrono_calc::Worksheet::Mortgage);
        QCOMPARE(boxes, (std::vector<std::size_t>{0, 1, 3, 4}));
        QCOMPARE(box(calculator, "Monthly payment"), QStringLiteral("1013,370619651761"));
        QVERIFY(choose(calculator, "Monthly payment"));
        QCOMPARE(result(calculator), QStringLiteral("1013,370619651761"));
        QCOMPARE(box(calculator, "Term (years)"), QStringLiteral("30"));
    }

    // Part B line 2: changing a box keeps the result; the result comes only with Calculate
    static void changingABoxKeepsTheResult()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        calculateFirstMortgage(calculator);
        QVERIFY(calculator.run("worksheet.value.2", L"15"));
        QCOMPARE(result(calculator), QStringLiteral("1013,370619651761"));
        QVERIFY(calculator.run("worksheet.calculate"));
        QCOMPARE(result(calculator), QStringLiteral("1529,986577626903"));
    }

    // Part B line 3: what a box takes as typed or pasted
    static void theBoxesTakeWhatWindows7Takes()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("view.fuel_economy_mpg"));
        const std::vector<std::pair<const char*, const char*>> typed = {
            {"abc", ""}, {"12a3", "123"}, {"1.5", "15"}, {"1 234", "1234"}, {"1e5", "1e5"}, {"+5", "+5"}, {"--5", "--5"},
            {"1,5,5", "1,5,5"}, {"1.234,5", "1234,5"}, {"12345678901234567890123456789", "1234567890123456789012345"}};
        for (const auto& [input, taken] : typed) {
            QVERIFY(calculator.run("worksheet.value.0", QString::fromUtf8(input).toStdWString()));
            QCOMPARE(box(calculator, "Distance (miles)"), QString::fromUtf8(taken));
        }
        // "," alone is 0
        QVERIFY(type(calculator, {",", "5"}));
        QVERIFY(calculator.run("worksheet.calculate"));
        QCOMPARE(outcome(calculator), QStringLiteral("0"));
        // The result box takes nothing typed
        QVERIFY(calculator.run("worksheet.value.2", L"9"));
        QCOMPARE(result(calculator), QStringLiteral("0"));
    }

    // Part D lines 2 and 3: the first box that is not valid wins over an empty one and keeps the result; then the first
    // empty box, which empties the result, as "Unable to calculate" does
    static void theMessagesComeInWindows7sOrder()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        calculateFirstMortgage(calculator);
        QVERIFY(type(calculator, {"", "50000", "-1", "4,5"}));
        QVERIFY(calculator.run("worksheet.calculate"));
        QCOMPARE(outcome(calculator), QStringLiteral("NV Term (years)"));
        QCOMPARE(result(calculator), QStringLiteral("1013,370619651761"));
        QCOMPARE(box(calculator, "Term (years)"), QStringLiteral("-1"));
        QVERIFY(type(calculator, {"250000", "", "30", ""}));
        QVERIFY(calculator.run("worksheet.calculate"));
        QCOMPARE(outcome(calculator), QStringLiteral("Blank Down payment"));
        QCOMPARE(result(calculator), QString());
        QVERIFY(type(calculator, {"", "", "", ""}));
        QVERIFY(calculator.run("worksheet.calculate"));
        QCOMPARE(outcome(calculator), QStringLiteral("Blank Purchase price"));
        QVERIFY(type(calculator, {"250000", "50000", "0", "4,5"}));
        QVERIFY(calculator.run("worksheet.calculate"));
        QCOMPARE(outcome(calculator), QStringLiteral("UC"));
        QCOMPARE(result(calculator), QString());
        // The next command takes the message away
        QVERIFY(calculator.run("worksheet.value.0", L"1"));
        QCOMPARE(calculator.worksheetCalculator().state(chrono_calc::Worksheet::Mortgage).message, chrono_calc::WorksheetMessage::None);
    }

    // Part B line 6: each worksheet keeps its value, boxes and result over a view change, another panel and Basic
    static void eachWorksheetKeepsItsState()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        calculateFirstMortgage(calculator);
        QVERIFY(choose(calculator, "Down payment"));
        QVERIFY(calculator.run("view.fuel_economy_litres"));
        QVERIFY(type(calculator, {"100", "3"}));
        QVERIFY(calculator.run("worksheet.calculate"));
        for (const char* command : {"view.scientific", "view.programmer", "view.statistics", "view.date_calculation", "view.basic",
                                    "view.standard", "view.mortgage"}) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(result(calculator), QStringLiteral("50000"));
        QCOMPARE(box(calculator, "Monthly payment"), QStringLiteral("1013,370619651761"));
        QVERIFY(calculator.run("view.fuel_economy_litres"));
        QCOMPARE(result(calculator), QStringLiteral("3"));
    }

    // Without a worksheet shown the commands change nothing
    static void theCommandsNeedAWorksheet()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("worksheet.value.0", L"5"));
        QVERIFY(calculator.run("worksheet.calculate"));
        QVERIFY(calculator.run("view.mortgage"));
        QCOMPARE(box(calculator, "Purchase price"), QString());
        QCOMPARE(calculator.worksheetCalculator().state(chrono_calc::Worksheet::Mortgage).message, chrono_calc::WorksheetMessage::None);
        QCOMPARE(text(calculator.display()), QStringLiteral("0"));
    }

    // D-027: a rate near 0, where 1 - (1 + i)^-n cancels, keeps its digits (worked as e^-(n ln(1 + i)) with series near
    // 0; review round 3: the engine's power takes 1 + 1e-65 as 1)
    static void aResultIsNeverShownWrong()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("view.mortgage"));
        QVERIFY(type(calculator, {"250000", "50000", "30", "1e-20"}));
        QVERIFY(calculator.run("worksheet.calculate"));
        QCOMPARE(outcome(calculator), QStringLiteral("5,555555555555556e+2"));
        QVERIFY(type(calculator, {"250000", "50000", "30", "1e-9999"}));
        QVERIFY(calculator.run("worksheet.calculate"));
        QCOMPARE(outcome(calculator), QStringLiteral("5,555555555555556e+2"));
        const std::vector<Line> lines = {
            {"tiny 1", mortgage, "Purchase price", {"0", "1", "1e-65", "100"}, "1,2e+3"},
            {"tiny 2", mortgage, "Down payment", {"5000", "1", "1e-100", "100"}, "3,8e+3"},
            {"tiny 3", lease, "Lease value", {"1", "12", "1000", "1e-70", "100"}, "2,2e+3"},
            {"tiny 4", lease, "Residual value", {"2200", "1", "12", "1e-100", "100"}, "1,e+3"},
            {"tiny 5", mortgage, "Term (years)", {"250000", "50000", "1e-30", "1000"}, "1,666666666666667e+1"},
            {"many 3", lease, "Residual value", {"1000", "300000000", "12", "0,000000001", "0"}, "1003,004504503376"},
        };
        for (const Line& line : lines) {
            QCOMPARE(calculated(line), QString::fromUtf8(line.result));
        }
    }

    // Found in review: a result below 1 written with an exponent keeps its 16 digits (the engine's own exponent form
    // counts the zeros after the decimal symbol); a payment over 10 million years still has its answer, the interest
    // only; where the tiny residual value now would be the whole answer, "Unable to calculate"
    static void extremeResultsKeepTheirDigits()
    {
        const std::vector<Line> lines = {
            {"small 1", mpg, "Fuel economy (mpg)", {"2", "3e20"}, "6,666666666666667e-21"},
            {"small 2", mpg, "Fuel economy (mpg)", {"1e-1", "7"}, "1,428571428571429e-2"},
            {"small 3", mpg, "Fuel economy (mpg)", {"1e-5", "7"}, "1,428571428571429e-6"},
            {"small 4", mortgage, "Monthly payment", {"1e1", "0", "30", "4,5"}, "5,066853098258807e-2"},
            {"long 1", mortgage, "Monthly payment", {"250000", "50000", "10000000", "4,5"}, "750"},
            {"long 2", mortgage, "Monthly payment", {"250000", "50000", "9999999999999999", "4,5"}, "750"},
            {"long 3", mortgage, "Down payment", {"250000", "1e8", "4,5", "1000"}, "-1,666666666666667e+4"},
            {"long 4", lease, "Periodic payment", {"30000", "1e8", "12", "1000", "5"}, "1,25e+2"},
            {"long 5", lease, "Periodic payment", {"0", "1e8", "12", "1000", "5"}, "UC"},
            {"long 6", lease, "Lease value", {"1e8", "12", "1000", "5", "0"}, "UC"},
            {"long 7", lease, "Lease value", {"1e8", "12", "1000", "5", "100"}, "2,4e+4"},
            // More than 2^31 payments at a tiny rate: (1 + i)^-n is near 1, not negligible (review round 2)
            {"many 1", mortgage, "Monthly payment", {"100000", "0", "200000000", "0,000000001"}, "4,170834722223866e-5"},
            {"many 2", lease, "Lease value", {"200000000", "12", "1000", "0,000000001", "1"}, "2397602597,201321"},
            // A result beyond 10^43429 written with an exponent (it ended the program before, review round 2)
            {"huge 1", lease, "Residual value", {"9999999999999999e9999", "104000", "12", "100", "0"}, "1,284137404991501e+53398"},
        };
        for (const Line& line : lines) {
            QCOMPARE(calculated(line), QString::fromUtf8(line.result));
        }
    }

    // The number library's radix and precision are global (D-025): after a Calculate the view's own are back
    static void theViewKeepsItsOwnPrecisionAndBase()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        for (const char* command : {"view.programmer", "base.hexadecimal", "view.mortgage", "digit.a", "add"}) {
            QVERIFY(calculator.run(command));
        }
        QVERIFY(type(calculator, {"250000", "50000", "30", "4,5"}));
        QVERIFY(calculator.run("worksheet.calculate"));
        for (const char* command : {"digit.1", "equals"}) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(text(calculator.display()), QStringLiteral("B"));
        for (const char* command : {"view.standard", "digit.1", "divide", "digit.3"}) {
            QVERIFY(calculator.run(command));
        }
        QVERIFY(calculator.run("worksheet.calculate"));
        QVERIFY(calculator.run("equals"));
        QCOMPARE(text(calculator.display()), QStringLiteral("0,3333333333333333"));
    }
};

QTEST_GUILESS_MAIN(TestReferenceWorksheets)
#include "test_reference_worksheets.moc"
