#include <QObject>
#include <QString>
#include <QTest>

#include "core/calculator.h"
#include "core/commands.h"
#include "core/enginetexts.h"
#include "core/unitconversion.h"
#include "core/unitconverter.h"
#include "core/view.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// --- Version ------------------------------------------------------------
// tests/test_reference_unitconversion.cpp 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Windows 7 Unit conversion results and behaviours of tests/reference/windows7-unitconversion.md
//          (M6)

// The lines of tests/reference/windows7-unitconversion.md, measured in the Windows 7 Calculator with the regional
// decimal symbol ','. Where Chrono Calc differs (the Energy and Power rows and two Pressure cells of part C1, line 31
// of part C2, D-025), the test holds Chrono Calc's value and says Windows 7's.
namespace {

constexpr chrono_calc::Separators commaSeparators{.decimal = L',', .thousands = L'.'};

struct FirstUnitRow
{
    const char* type;
    std::vector<const char*> results;
};

struct Conversion
{
    const char* name;
    const char* value;
    const char* from;
    const char* to;
    const char* result;
};

// Part C1: 1 of the type's first unit in every unit of its list
std::vector<FirstUnitRow> firstUnitRows()
{
    return {
        {"Angle", {"1", "1,111111111111111", "0,0174532925199433"}},
        {"Area", {"1", "0,40468564224", "40468564,224", "43560", "6272640", "0,0040468564224", "4046,8564224", "0,0015625", "4046856422,4", "4840"}},
        {"Energy", {"1", "251,9957611111111", "6,585140678190667e+21", "778,1692622659649", "1055,05585262", "0,2519957611111111", "1,05505585262"}},
        {"Length", {"1", "0,00000001", "4,970969537898672e-12", "5,468066491688539e-11", "3,280839895013123e-10", "9,84251968503937e-10", "3,937007874015748e-9", "0,0000000000001", "4,970969537898672e-10", "0,0000000001", "0,0001", "6,21371192237334e-14", "0,0000001", "0,1", "5,399568034557235e-14", "2,371063015836614e-8", "1,988387815159469e-11", "4,374453193350831e-10", "1,093613298337708e-10"}},
        {"Power", {"1", "778,1692622659649", "0,0235808867353323", "0,0175842642103333", "17,58426421033333"}},
        {"Pressure", {"1", "1,01325", "101,325", "760", "101325", "14,69594877551345"}},
        {"Temperature", {"1", "33,8", "274,15"}},
        {"Time", {"1", "24", "86400000000", "86400000", "1440", "86400", "0,1428571428571429"}},
        {"Velocity", {"1", "0,0328083989501312", "0,036", "0,019438444924406", "2,938641460175678e-5", "0,01", "0,022369362920544"}},
        {"Volume", {"1", "3,531466672148859e-5", "0,0610237440947323", "0,000001", "1,307950619314392e-6", "0,035195079727854", "0,033814022701843", "2,199692482990878e-4", "2,641720523581484e-4", "0,001", "0,0017597539863927", "0,0021133764188652", "8,798769931963512e-4", "0,0010566882094326"}},
        {"Weight/Mass", {"1", "20", "2", "0,02", "0,2", "0,002", "0,0002", "1,968413055222121e-7", "200", "0,0070547923899161", "4,409245243697552e-4", "2,204622621848776e-7", "3,149460888355394e-5", "0,0000002"}},
    };
}

// Part C2
std::vector<Conversion> conversions()
{
    return {
        {"C2 1", "0", "Degrees Celsius", "Degrees Fahrenheit", "32"},
        {"C2 2", "0", "Degrees Celsius", "Kelvin", "273,15"},
        {"C2 3", "100", "Degrees Fahrenheit", "Degrees Celsius", "37,77777777777778"},
        {"C2 4", "100", "Degrees Fahrenheit", "Kelvin", "310,9277777777778"},
        {"C2 5", "-40", "Degrees Celsius", "Degrees Fahrenheit", "-40"},
        {"C2 6", "1", "Kelvin", "Degrees Celsius", "-272,15"},
        {"C2 7", "0", "Kelvin", "Degrees Fahrenheit", "-459,67"},
        {"C2 8", "-500", "Degrees Celsius", "Kelvin", "-226,85"},
        {"C2 9", "1", "Inch", "Centimeters", "2,54"},
        {"C2 10", "1", "Mile", "Kilometers", "1,609344"},
        {"C2 11", "1", "Nautical Miles", "Meter", "1852"},
        {"C2 12", "3", "Feet", "Yard", "1"},
        {"C2 13", "1", "Angstrom", "Meter", "0,0000000001"},
        {"C2 14", "123456789", "Kilometers", "Inch", "4860503503937,008"},
        {"C2 15", "1e30", "Angstrom", "Mile", "6,21371192237334e+16"},
        {"C2 16", "1e30", "Mile", "Angstrom", "1,609344e+43"},
        {"C2 17", "1e-30", "Angstrom", "Mile", "6,21371192237334e-44"},
        {"C2 18", "1", "Pound", "Kilogram", "0,45359237"},
        {"C2 19", "1", "Ounce", "Gram", "28,349523125"},
        {"C2 20", "1", "Carat", "Gram", "0,2"},
        {"C2 21", "1", "Stone", "Pound", "14"},
        {"C2 22", "1", "Short ton", "Kilogram", "907,18474"},
        {"C2 23", "1", "Long ton", "Kilogram", "1016,0469088"},
        {"C2 24", "1", "Gallon (US)", "Liter", "3,785411784"},
        {"C2 25", "1", "Gallon (UK)", "Liter", "4,54609"},
        {"C2 26", "1", "Cubic feet", "Liter", "28,316846592"},
        {"C2 27", "1", "Week", "Second", "604800"},
        {"C2 28", "1", "Day", "Microsecond", "86400000000"},
        {"C2 29", "180", "Degree", "Radian", "3,141592653589793"},
        {"C2 30", "180", "Degree", "Gradian", "200"},
        // Windows 7: 57,29577951308233; 180/pi is 57,295779513082320877 (D-025)
        {"C2 31 Chrono Calc", "1", "Radian", "Degree", "57,29577951308232"},
        {"C2 32", "1", "Calorie", "Joule", "4,1868"},
        {"C2 33", "1", "Horsepower", "Watt", "745,6998715822702"},
        {"C2 34", "1", "Atmosphere", "Pascal", "101325"},
        {"C2 35", "1", "Knots", "Kilometer per hour", "1,852"},
        {"C2 36", "1", "Acres", "Square meters", "4046,8564224"},
        {"C2 37", "1", "Hectares", "Acres", "2,471053814671653"},
        {"C2 38", "1", "Feet", "Yard", "0,3333333333333333"},
        {"C2 39", "1", "Yard", "Feet", "3"},
        {"C2 40", "1,e+30", "Kilometers", "Inch", "3,937007874015748e+34"},
        {"C2 41", "1", "Inch", "Chain", "0,0012626262626263"},
        {"C2 42", "1", "Kilometers", "Chain", "49,70969537898672"},
        {"C2 43", "5", "Inch", "Centimeters", "12,7"},
    };
}

QString text(const std::wstring& value)
{
    return QString::fromStdWString(value);
}

// The type and unit index of a unit's name (every name is used once over all types)
std::optional<std::pair<std::size_t, std::size_t>> unitOf(std::string_view name)
{
    const std::vector<chrono_calc::UnitTypeSpec>& types = chrono_calc::unitTypes();
    for (std::size_t type = 0; type < types.size(); ++type) {
        for (std::size_t unit = 0; unit < types[type].units.size(); ++unit) {
            if (types[type].units[unit].name == name) {
                return std::pair{type, unit};
            }
        }
    }
    return std::nullopt;
}

std::optional<std::size_t> typeOf(std::string_view name)
{
    const std::vector<chrono_calc::UnitTypeSpec>& types = chrono_calc::unitTypes();
    for (std::size_t type = 0; type < types.size(); ++type) {
        if (types[type].name == name) {
            return type;
        }
    }
    return std::nullopt;
}

bool run(chrono_calc::Calculator& calculator, const std::string& commandId)
{
    return calculator.run(commandId);
}

// Converts as the panel does: the type, From's unit, To's unit, then the From box typed; the three are written in the
// reference's order (value, From, To)
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters): a conversion as the reference writes it
std::optional<QString> converted(chrono_calc::Calculator& calculator, const char* value, const char* from, const char* to)
{
    const auto fromUnit = unitOf(from);
    const auto toUnit = unitOf(to);
    if (!fromUnit || !toUnit || fromUnit->first != toUnit->first) {
        return std::nullopt;
    }
    if (!run(calculator, "unit.type." + std::to_string(fromUnit->first)) || !run(calculator, "unit.from." + std::to_string(fromUnit->second))
        || !run(calculator, "unit.to." + std::to_string(toUnit->second))
        || !calculator.run("unit.input", QString::fromUtf8(value).toStdWString())) {
        return std::nullopt;
    }
    return text(calculator.unitConverter().result());
}

QString shownInput(const chrono_calc::Calculator& calculator)
{
    return text(calculator.unitConverter().input());
}

}  // namespace

class TestReferenceUnitConversion : public QObject
{
    Q_OBJECT

private slots:
    // Part B: the types and their units, in Windows 7's order and spelling
    static void theListsAreWindows7s()
    {
        const std::vector<std::pair<const char*, std::vector<const char*>>> expected = {
            {"Angle", {"Degree", "Gradian", "Radian"}},
            {"Area", {"Acres", "Hectares", "Square centimeter", "Square feet", "Square inch", "Square kilometer", "Square meters",
                      "Square mile", "Square millimeter", "Square Yard"}},
            {"Energy", {"British Thermal Unit", "Calorie", "Electron-Volts", "Foot-Pound", "Joule", "Kilocalorie", "Kilojoule"}},
            {"Length", {"Angstrom", "Centimeters", "Chain", "Fathom", "Feet", "Hand", "Inch", "Kilometers", "Link", "Meter",
                        "Microns", "Mile", "Millimeters", "Nanometer", "Nautical Miles", "PICA", "Rods", "Span", "Yard"}},
            {"Power", {"BTU/minute", "Foot-Pound/minute", "Horsepower", "Kilowatt", "Watt"}},
            {"Pressure", {"Atmosphere", "Bar", "Kilo Pascal", "Millimeter of mercury", "Pascal", "Pound per square inch (PSI)"}},
            {"Temperature", {"Degrees Celsius", "Degrees Fahrenheit", "Kelvin"}},
            {"Time", {"Day", "Hour", "Microsecond", "Millisecond", "Minute", "Second", "Week"}},
            {"Velocity", {"Centimeter per second", "Feet per second", "Kilometer per hour", "Knots", "Mach (at std. atm)",
                          "Meter per second", "Miles per hour"}},
            {"Volume", {"Cubic centimeter", "Cubic feet", "Cubic inch", "Cubic meter", "Cubic yard", "Fluid ounce (UK)",
                        "Fluid ounce (US)", "Gallon (UK)", "Gallon (US)", "Liter", "Pint (UK)", "Pint (US)", "Quart (UK)", "Quart (US)"}},
            {"Weight/Mass", {"Carat", "Centigram", "Decigram", "Dekagram", "Gram", "Hectogram", "Kilogram", "Long ton", "Milligram",
                             "Ounce", "Pound", "Short ton", "Stone", "Tonne"}},
        };
        const std::vector<chrono_calc::UnitTypeSpec>& types = chrono_calc::unitTypes();
        QCOMPARE(types.size(), expected.size());
        for (std::size_t type = 0; type < types.size(); ++type) {
            QCOMPARE(std::string(types[type].name), std::string(expected[type].first));
            QCOMPARE(types[type].units.size(), expected[type].second.size());
            for (std::size_t unit = 0; unit < types[type].units.size(); ++unit) {
                QCOMPARE(std::string(types[type].units[unit].name), std::string(expected[type].second[unit]));
            }
        }
    }

    // Part C1
    static void oneOfTheFirstUnitInEveryUnit_data()
    {
        QTest::addColumn<int>("row");
        const std::vector<FirstUnitRow> rows = firstUnitRows();
        for (std::size_t row = 0; row < rows.size(); ++row) {
            QTest::newRow(rows[row].type) << static_cast<int>(row);
        }
    }

    static void oneOfTheFirstUnitInEveryUnit()
    {
        QFETCH(int, row);  // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        const FirstUnitRow line = firstUnitRows()[static_cast<std::size_t>(row)];
        const std::size_t type = typeOf(line.type).value_or(chrono_calc::unitTypes().size());
        QVERIFY(type < chrono_calc::unitTypes().size());
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(run(calculator, "unit.type." + std::to_string(type)));
        QVERIFY(calculator.run("unit.input", L"1"));
        QCOMPARE(chrono_calc::unitTypes()[type].units.size(), line.results.size());
        for (std::size_t unit = 0; unit < line.results.size(); ++unit) {
            QVERIFY(run(calculator, "unit.to." + std::to_string(unit)));
            QCOMPARE(text(calculator.unitConverter().result()), QString::fromUtf8(line.results[unit]));
        }
    }

    // Part C2
    static void otherResults_data()
    {
        QTest::addColumn<int>("line");
        const std::vector<Conversion> lines = conversions();
        for (std::size_t line = 0; line < lines.size(); ++line) {
            QTest::newRow(lines[line].name) << static_cast<int>(line);
        }
    }

    static void otherResults()
    {
        QFETCH(int, line);  // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        const Conversion conversion = conversions()[static_cast<std::size_t>(line)];
        chrono_calc::Calculator calculator(commaSeparators);
        QCOMPARE(converted(calculator, conversion.value, conversion.from, conversion.to), std::optional(QString::fromUtf8(conversion.result)));
    }

    // Part C line 2: the To box is never grouped
    static void theResultIsNeverGrouped()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("view.digit_grouping"));
        QVERIFY(calculator.isDigitGrouping());
        QCOMPARE(converted(calculator, "123456789", "Kilometers", "Inch"), std::optional(QStringLiteral("4860503503937,008")));
    }

    // The cells where Windows 7's rounded factors are not copied (D-025); Windows 7 showed the second value
    static void windows7sRoundedFactorsAreNotCopied()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QCOMPARE(converted(calculator, "1", "Atmosphere", "Millimeter of mercury"), std::optional(QStringLiteral("760")));  // 760,1275318829707
        QCOMPARE(converted(calculator, "1", "Atmosphere", "Pound per square inch (PSI)"), std::optional(QStringLiteral("14,69594877551345")));  // 14,69594940039221
        QCOMPARE(converted(calculator, "1", "British Thermal Unit", "Electron-Volts"), std::optional(QStringLiteral("6,585140678190667e+21")));  // 6,585142025517001e+21
        QCOMPARE(converted(calculator, "1", "British Thermal Unit", "Foot-Pound"), std::optional(QStringLiteral("778,1692622659649")));  // 778,1693709678747
        QCOMPARE(converted(calculator, "1", "BTU/minute", "Foot-Pound/minute"), std::optional(QStringLiteral("778,1692622659649")));  // 778,1693709678747
        QCOMPARE(converted(calculator, "1", "British Thermal Unit", "Calorie"), std::optional(QStringLiteral("251,9957611111111")));  // 251,9957963122194
        QCOMPARE(converted(calculator, "1", "Foot-Pound", "Joule"), std::optional(QStringLiteral("1,3558179483314")));
    }

    // Part G: From read as Windows 7 reads it, and a To written with an exponent when From has one
    static void fromIsReadAsWindows7ReadsIt()
    {
        const std::vector<std::pair<const char*, const char*>> lines = {
            {"00000000000000001", "100"}, {"e5", "1,e+7"}, {"1e", "1,e+2"}, {",", "0"}, {"-", "0"}, {"+", "0"}, {"-0", "0"},
            {"0,0000000000000000001", ""}, {"1e400", "1,e+402"}, {"1e10000", ""}};
        chrono_calc::Calculator calculator(commaSeparators);
        for (const auto& [from, to] : lines) {
            QCOMPARE(converted(calculator, from, "Meter", "Centimeters"), std::optional(QString::fromUtf8(to)));
        }
        // An exponent with a result below 1 keeps its 16 digits
        QCOMPARE(converted(calculator, "1e-1", "Centimeters", "Inch"), std::optional(QStringLiteral("3,937007874015748e-2")));
    }

    // Part B lines 1 and 2: the first time Angle with Degree in both lists and empty boxes; a type chosen sets both
    // units to its first one and empties the boxes, every time
    static void choosingATypeStartsItAgain()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        const chrono_calc::UnitConverter& converter = calculator.unitConverter();
        QCOMPARE(converter.type(), std::size_t{0});
        QCOMPARE(converter.from(), std::size_t{0});
        QCOMPARE(converter.to(), std::size_t{0});
        QVERIFY(converter.input().empty());
        QVERIFY(converter.result().empty());
        QCOMPARE(converted(calculator, "5", "Inch", "Centimeters"), std::optional(QStringLiteral("12,7")));
        QVERIFY(calculator.run("unit.type.3"));
        QCOMPARE(converter.type(), std::size_t{3});
        QCOMPARE(converter.from(), std::size_t{0});
        QCOMPARE(converter.to(), std::size_t{0});
        QVERIFY(converter.input().empty());
        QVERIFY(converter.result().empty());
        // A type or unit beyond the lists changes nothing
        QVERIFY(calculator.run("unit.input", L"2"));
        QVERIFY(calculator.run("unit.type.11"));
        QVERIFY(calculator.run("unit.from.19"));
        QCOMPARE(converter.type(), std::size_t{3});
        QCOMPARE(shownInput(calculator), QStringLiteral("2"));
    }

    // Part D line 5: a unit chosen keeps From and calculates To at once (part C2 lines 41 and 42)
    static void choosingAUnitKeepsFrom()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QCOMPARE(converted(calculator, "1", "Inch", "Centimeters"), std::optional(QStringLiteral("2,54")));
        QVERIFY(calculator.run("unit.to." + std::to_string(unitOf("Chain").value_or(std::pair{0, 0}).second)));
        QCOMPARE(text(calculator.unitConverter().result()), QStringLiteral("0,0012626262626263"));
        QVERIFY(calculator.run("unit.from." + std::to_string(unitOf("Kilometers").value_or(std::pair{0, 0}).second)));
        QCOMPARE(shownInput(calculator), QStringLiteral("1"));
        QCOMPARE(text(calculator.unitConverter().result()), QStringLiteral("49,70969537898672"));
    }

    // Part D line 2: what the From box takes as it is typed, and which texts give a result (Meter to Centimeters)
    static void theFromBoxTakesWindows7sCharacters_data()
    {
        QTest::addColumn<QString>("typed");
        QTest::addColumn<QString>("taken");
        QTest::addColumn<QString>("result");
        QTest::newRow("abc") << "abc" << "" << "";
        QTest::newRow("12a3") << "12a3" << "123" << "12300";
        QTest::newRow("1E30") << "1E30" << "130" << "13000";
        QTest::newRow("1.5") << "1.5" << "15" << "1500";
        QTest::newRow("1.234,5") << "1.234,5" << "1234,5" << "123450";
        QTest::newRow(" 2 ") << " 2 " << "2" << "200";
        QTest::newRow("-5") << "-5" << "-5" << "-500";
        QTest::newRow("+5") << "+5" << "+5" << "500";
        QTest::newRow("-0,5") << "-0,5" << "-0,5" << "-50";
        QTest::newRow("1,5") << "1,5" << "1,5" << "150";
        QTest::newRow("-3") << "-3" << "-3" << "-300";
        QTest::newRow("5-") << "5-" << "5-" << "";
        QTest::newRow("--5") << "--5" << "--5" << "";
        QTest::newRow("1,5,5") << "1,5,5" << "1,5,5" << "";
        QTest::newRow("1e30") << "1e30" << "1e30" << "1,e+32";
        QTest::newRow("16 digits") << "1234567890123456" << "1234567890123456" << "1,234567890123456e+17";
        QTest::newRow("17 digits") << "12345678901234567" << "12345678901234567" << "";
        QTest::newRow("26 characters") << "12345678901234567890123456" << "1234567890123456789012345" << "";
    }

    static void theFromBoxTakesWindows7sCharacters()
    {
        QFETCH(QString, typed);  // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        QFETCH(QString, taken);  // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        QFETCH(QString, result);  // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        chrono_calc::Calculator calculator(commaSeparators);
        QCOMPARE(converted(calculator, "", "Meter", "Centimeters"), std::optional(QString()));
        QVERIFY(calculator.run("unit.input", typed.toStdWString()));
        QCOMPARE(shownInput(calculator), taken);
        QCOMPARE(text(calculator.unitConverter().result()), result);
    }

    // Part D line 1: the From box and the display never change each other
    static void theDisplayAndTheFromBoxAreApart()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("view.unit_conversion"));
        QCOMPARE(converted(calculator, "5", "Inch", "Centimeters"), std::optional(QStringLiteral("12,7")));
        QCOMPARE(text(calculator.display()), QStringLiteral("0"));
        for (const char* command : {"digit.4", "digit.2", "add", "digit.2", "equals", "clear_entry", "digit.1", "divide", "digit.0", "equals"}) {
            QVERIFY(calculator.run(command));
        }
        QVERIFY(calculator.isError());
        QCOMPARE(shownInput(calculator), QStringLiteral("5"));
        QCOMPARE(text(calculator.unitConverter().result()), QStringLiteral("12,7"));
    }

    // Part A lines 1 and 4, part D line 9: the panel shows beside every view and keeps its state over a view change;
    // Basic hides it and keeps the state; choosing it again changes nothing
    static void thePanelStaysOverViewChanges()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QCOMPARE(calculator.sidePanel(), chrono_calc::SidePanel::Basic);
        QVERIFY(calculator.run("view.unit_conversion"));
        QCOMPARE(calculator.sidePanel(), chrono_calc::SidePanel::UnitConversion);
        QCOMPARE(converted(calculator, "5", "Inch", "Centimeters"), std::optional(QStringLiteral("12,7")));
        for (const char* view : {"view.scientific", "view.programmer", "view.statistics", "view.standard", "view.unit_conversion"}) {
            QVERIFY(calculator.run(view));
            QCOMPARE(calculator.sidePanel(), chrono_calc::SidePanel::UnitConversion);
            QCOMPARE(shownInput(calculator), QStringLiteral("5"));
            QCOMPARE(text(calculator.unitConverter().result()), QStringLiteral("12,7"));
        }
        QVERIFY(calculator.run("view.basic"));
        QCOMPARE(calculator.sidePanel(), chrono_calc::SidePanel::Basic);
        QVERIFY(calculator.run("view.unit_conversion"));
        QCOMPARE(calculator.unitConverter().type(), unitOf("Inch").value_or(std::pair{0, 0}).first);
        QCOMPARE(text(calculator.unitConverter().result()), QStringLiteral("12,7"));
    }

    // The Programmer view's engine works in its own base; a conversion is decimal, and the engine keeps its base
    static void aConversionInTheProgrammerViewLeavesItsBase()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        for (const char* command : {"view.programmer", "base.hexadecimal", "view.unit_conversion"}) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(converted(calculator, "180", "Degree", "Radian"), std::optional(QStringLiteral("3,141592653589793")));
        for (const char* command : {"digit.f", "add", "digit.1", "equals"}) {
            QVERIFY(calculator.run(command));
        }
        QCOMPARE(text(calculator.display()), QStringLiteral("10"));
    }

    // The ids that carry a list's item
    static void theUnitCommandsCarryTheirItem()
    {
        QCOMPARE(chrono_calc::findCommand("unit.type.7").value_or(chrono_calc::CommandTarget{}).kind, chrono_calc::CommandKind::UnitType);
        QCOMPARE(chrono_calc::findCommand("unit.from.12").value_or(chrono_calc::CommandTarget{}).index, std::size_t{12});
        QCOMPARE(chrono_calc::findCommand("unit.to.3").value_or(chrono_calc::CommandTarget{}).kind, chrono_calc::CommandKind::UnitTo);
        for (const char* wrong : {"unit.type.", "unit.from.x", "unit.to.-1", "unit.type.1234567890", "unit.type"}) {
            QVERIFY2(!chrono_calc::findCommand(wrong).has_value(), wrong);
        }
    }
};

QTEST_GUILESS_MAIN(TestReferenceUnitConversion)
#include "test_reference_unitconversion.moc"
