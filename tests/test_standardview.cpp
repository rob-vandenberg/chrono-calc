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
// tests/test_standardview.cpp 0.2.7

// --- Version history ----------------------------------------------------
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
            QVERIFY(!overlap(place, layout.angleFrame));
            QVERIFY(!overlap(place, layout.bracketCounter));
            for (std::size_t other = index + 1; other < buttons.size(); ++other) {
                QVERIFY2(!overlap(place, buttons[other].rectangle), button.commandId.data());
            }
        }
        for (const chrono_calc::AngleSpec& angle : layout.angles) {
            QVERIFY(chrono_calc::findCommand(angle.commandId).has_value());
        }
    }

    // The Scientific measures of Windows 7 (part A of windows7-scientific.md)
    static void theScientificViewHasTheMeasuresOfWindows7()
    {
        const chrono_calc::ViewLayout& layout = chrono_calc::layoutOf(chrono_calc::View::Scientific);
        QCOMPARE(layout.clientWidth, 508);
        QCOMPARE(layout.clientHeight, 331);
        QCOMPARE(layout.displayFrame.width, 482);
        QCOMPARE(layout.angles.size(), std::size_t{3});
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

    // Windows 7 greys % in the Scientific view; every other button can be used
    static void onlyPercentIsGreyedInScientific()
    {
        for (const chrono_calc::ButtonSpec& button : chrono_calc::layoutOf(chrono_calc::View::Scientific).buttons) {
            QCOMPARE(chrono_calc::Calculator::isAvailable(chrono_calc::View::Scientific, button.commandId), button.commandId != "percent");
        }
        for (const chrono_calc::ButtonSpec& button : chrono_calc::layoutOf(chrono_calc::View::Standard).buttons) {
            QVERIFY(chrono_calc::Calculator::isAvailable(chrono_calc::View::Standard, button.commandId));
        }
    }

    // The faces of Windows 7: digits and the decimal point light, the memory row flat, Inv and F-E toggles
    static void everyButtonHasTheFaceOfWindows7()
    {
        for (const chrono_calc::View view : {chrono_calc::View::Standard, chrono_calc::View::Scientific}) {
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
        QCOMPARE(built, 3);
        QCOMPARE(greyed, 18);
        const auto view = chrono_calc::standardMenus().front();
        QCOMPARE(view.items.front().entry.commandId, std::string_view("view.standard"));
        QVERIFY(view.items.front().entry.chosen);
    }
};

QTEST_GUILESS_MAIN(TestStandardView)
#include "test_standardview.moc"
