#include <QObject>
#include <QTest>

#include "core/commands.h"
#include "core/menus.h"
#include "core/calculator.h"
#include "core/layout.h"
#include "core/view.h"

#include <cstddef>
#include <set>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// tests/test_standardview.cpp 0.3.8

// --- Version history ----------------------------------------------------
// v0.3.8: The Programmer view: its 45 buttons, choices and the bits; what each base allows
// v0.2.7: Both views: Standard and Scientific (the Windows 7 measures of windows7-scientific.md part A), the faces
// v0.1.6: Initial: the Standard view's buttons and menus (core): every one runs a registered command

namespace {

bool overlap(chrono_calc::Rectangle first, chrono_calc::Rectangle second)
{
    return first.left < second.left + second.width && second.left < first.left + first.width
           && first.top < second.top + second.height && second.top < first.top + first.height;
}

}  // namespace

class TestStandardView : public QObject
{
    Q_OBJECT

private slots:
    // Every button of a view runs its own registered command, inside the window and on top of no other part
    static void everyButtonRunsARegisteredCommandInItsOwnPlace_data()
    {
        QTest::addColumn<int>("view");
        QTest::addColumn<int>("count");
        QTest::newRow("Standard") << static_cast<int>(chrono_calc::View::Standard) << 28;
        QTest::newRow("Scientific") << static_cast<int>(chrono_calc::View::Scientific) << 52;
        QTest::newRow("Programmer") << static_cast<int>(chrono_calc::View::Programmer) << 45;
    }

    static void everyButtonRunsARegisteredCommandInItsOwnPlace()
    {
        QFETCH(int, view);   // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        QFETCH(int, count);  // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        const chrono_calc::ViewLayout& layout = chrono_calc::layoutOf(static_cast<chrono_calc::View>(view));
        const auto buttons = layout.buttons;
        QCOMPARE(buttons.size(), static_cast<std::size_t>(count));
        std::set<std::string_view> commands;
        for (std::size_t index = 0; index < buttons.size(); ++index) {
            const chrono_calc::ButtonSpec& button = buttons[index];
            QVERIFY2(chrono_calc::findCommand(button.commandId).has_value(), button.commandId.data());
            QVERIFY2(commands.insert(button.commandId).second, button.commandId.data());
            QVERIFY(!button.label.empty());
            const chrono_calc::Rectangle place = button.rectangle;
            QVERIFY(place.left >= 0 && place.top >= 0);
            QVERIFY(place.left + place.width <= layout.clientWidth);
            QVERIFY(place.top + place.height <= layout.clientHeight);
            QVERIFY(!overlap(place, layout.displayFrame));
            for (const chrono_calc::Rectangle& frame : layout.frames) {
                QVERIFY(!overlap(place, frame));
            }
            QVERIFY(!overlap(place, layout.bitDisplay.frame));
            QVERIFY(!overlap(place, layout.bracketCounter));
            for (std::size_t other = index + 1; other < buttons.size(); ++other) {
                QVERIFY2(!overlap(place, buttons[other].rectangle), button.commandId.data());
            }
        }
        for (const chrono_calc::ChoiceSpec& choice : layout.choices) {
            QVERIFY(chrono_calc::findCommand(choice.commandId).has_value());
        }
    }

    // The Scientific measures of Windows 7 (part A of windows7-scientific.md)
    static void theScientificViewHasTheMeasuresOfWindows7()
    {
        const chrono_calc::ViewLayout& layout = chrono_calc::layoutOf(chrono_calc::View::Scientific);
        QCOMPARE(layout.clientWidth, 508);
        QCOMPARE(layout.clientHeight, 331);
        QCOMPARE(layout.displayFrame.width, 482);
        QCOMPARE(layout.choices.size(), std::size_t{3});
        const auto place = [&layout](std::string_view commandId) {
            for (const chrono_calc::ButtonSpec& button : layout.buttons) {
                if (button.commandId == commandId) {
                    return button.rectangle;
                }
            }
            return chrono_calc::Rectangle{};
        };
        QCOMPARE(place("inverse").left, 62);
        QCOMPARE(place("inverse").top, 122);
        QCOMPARE(place("integer").left, 13);
        QCOMPARE(place("power_of_ten").left, 208);
        QCOMPARE(place("power_of_ten").top, 282);
        QCOMPARE(place("memory.clear").left, 257);
        QCOMPARE(place("percent").left, 452);
        QCOMPARE(place("equals").left, 452);
        QCOMPARE(place("equals").height, 74);
        QCOMPARE(place("digit.0").left, 257);
        QCOMPARE(place("digit.0").width, 92);
        QCOMPARE(place("decimal").left, 355);
    }

    // The Programmer measures of Windows 7 (windows7-programmer.md part A): the bits sit in groups of four
    static void theProgrammerViewHasTheMeasuresOfWindows7()
    {
        const chrono_calc::ViewLayout& layout = chrono_calc::layoutOf(chrono_calc::View::Programmer);
        QCOMPARE(layout.clientHeight, 413);
        QCOMPARE(layout.choices.size(), std::size_t{8});
        QCOMPARE(chrono_calc::bitRectangle(63).left, 22);
        QCOMPARE(chrono_calc::bitRectangle(63).top, 88);
        QCOMPARE(chrono_calc::bitRectangle(62).left, 32);
        QCOMPARE(chrono_calc::bitRectangle(60).left, 51);
        QCOMPARE(chrono_calc::bitRectangle(59).left, 82);
        QCOMPARE(chrono_calc::bitRectangle(32).left, 471);
        QCOMPARE(chrono_calc::bitRectangle(31).top, 121);
        QCOMPARE(chrono_calc::bitRectangle(0).left, 471);
        QCOMPARE(chrono_calc::bitRectangle(0).width, 9);
    }

    // What each base allows in Programmer (Windows 7): never the decimal point, sqrt, % and 1/x; A to F only in Hex,
    // 8 and 9 not in Oct, only 0 and 1 in Bin
    static void eachBaseAllowsItsOwnDigits()
    {
        using chrono_calc::Calculator;
        using chrono_calc::NumberBase;
        constexpr auto programmer = chrono_calc::View::Programmer;
        for (const NumberBase base : {NumberBase::Hexadecimal, NumberBase::Decimal, NumberBase::Octal, NumberBase::Binary}) {
            for (const char* never : {"decimal", "sqrt", "percent", "reciprocal", "sin", "inverse"}) {
                QVERIFY(!Calculator::isAvailable(programmer, base, never));
            }
            QVERIFY(Calculator::isAvailable(programmer, base, "digit.1"));
            QVERIFY(Calculator::isAvailable(programmer, base, "modulo"));
        }
        QVERIFY(Calculator::isAvailable(programmer, NumberBase::Hexadecimal, "digit.f"));
        QVERIFY(!Calculator::isAvailable(programmer, NumberBase::Decimal, "digit.a"));
        QVERIFY(Calculator::isAvailable(programmer, NumberBase::Decimal, "digit.9"));
        QVERIFY(!Calculator::isAvailable(programmer, NumberBase::Octal, "digit.8"));
        QVERIFY(Calculator::isAvailable(programmer, NumberBase::Octal, "digit.7"));
        QVERIFY(!Calculator::isAvailable(programmer, NumberBase::Binary, "digit.2"));
        QVERIFY(!Calculator::isAvailable(chrono_calc::View::Standard, NumberBase::Hexadecimal, "digit.a"));
        QVERIFY(!Calculator::isAvailable(chrono_calc::View::Scientific, NumberBase::Hexadecimal, "and"));
    }

    // Windows 7 greys % in the Scientific view; every other button can be used
    static void onlyPercentIsGreyedInScientific()
    {
        for (const chrono_calc::ButtonSpec& button : chrono_calc::layoutOf(chrono_calc::View::Scientific).buttons) {
            QCOMPARE(chrono_calc::Calculator::isAvailable(chrono_calc::View::Scientific, chrono_calc::NumberBase::Decimal, button.commandId), button.commandId != "percent");
        }
        for (const chrono_calc::ButtonSpec& button : chrono_calc::layoutOf(chrono_calc::View::Standard).buttons) {
            QVERIFY(chrono_calc::Calculator::isAvailable(chrono_calc::View::Standard, chrono_calc::NumberBase::Decimal, button.commandId));
        }
    }

    // The faces of Windows 7: digits and the decimal point light, the memory row flat, Inv and F-E toggles
    static void everyButtonHasTheFaceOfWindows7()
    {
        for (const chrono_calc::View view : {chrono_calc::View::Standard, chrono_calc::View::Scientific, chrono_calc::View::Programmer}) {
            for (const chrono_calc::ButtonSpec& button : chrono_calc::layoutOf(view).buttons) {
                const std::string id(button.commandId);
                chrono_calc::ButtonFace expected = chrono_calc::ButtonFace::Function;
                if (id.starts_with("digit.") || id == "decimal") {
                    expected = chrono_calc::ButtonFace::Digit;
                } else if (id.starts_with("memory.")) {
                    expected = chrono_calc::ButtonFace::Memory;
                } else if (id == "inverse" || id == "scientific_notation") {
                    expected = chrono_calc::ButtonFace::Toggle;
                }
                if (id.size() == 7 && id.starts_with("digit.") && id[6] >= 'a') {
                    expected = chrono_calc::ButtonFace::Flat;
                }
                QVERIFY2(button.face == expected, id.c_str());
            }
        }
    }

    // Every menu item with a command runs a registered one; View > Standard and Basic carry their dot
    static void everyBuiltMenuItemRunsARegisteredCommand()
    {
        int built = 0;
        int greyed = 0;
        const auto check = [&built, &greyed](const chrono_calc::MenuEntry& entry) {
            if (entry.commandId.empty()) {
                ++greyed;
                return;
            }
            ++built;
            QVERIFY2(chrono_calc::findCommand(entry.commandId).has_value(), entry.commandId.data());
        };
        for (const chrono_calc::Menu& menu : chrono_calc::standardMenus()) {
            for (const chrono_calc::MenuItem& item : menu.items) {
                if (item.submenu.empty()) {
                    check(item.entry);
                }
                for (const chrono_calc::MenuEntry& entry : item.submenu) {
                    check(entry);
                }
            }
        }
        QCOMPARE(built, 4);
        QCOMPARE(greyed, 17);
        const auto view = chrono_calc::standardMenus().front();
        QCOMPARE(view.items.front().entry.commandId, std::string_view("view.standard"));
        QVERIFY(view.items.front().entry.chosen);
    }
};

QTEST_GUILESS_MAIN(TestStandardView)
#include "test_standardview.moc"
