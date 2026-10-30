#include <QObject>
#include <QTest>

#include "core/commands.h"
#include "core/menus.h"
#include "core/standardlayout.h"

#include <cstddef>
#include <set>
#include <string_view>

// --- Version ------------------------------------------------------------
// tests/test_standardview.cpp 0.1.6

// --- Version history ----------------------------------------------------
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
    // The 28 buttons of Windows 7, each with its own registered command, inside the window and on top of no other
    static void everyButtonRunsARegisteredCommandInItsOwnPlace()
    {
        const auto buttons = chrono_calc::standard::buttons();
        QCOMPARE(buttons.size(), std::size_t{28});
        std::set<std::string_view> commands;
        for (std::size_t index = 0; index < buttons.size(); ++index) {
            const chrono_calc::ButtonSpec& button = buttons[index];
            QVERIFY2(chrono_calc::findCommand(button.commandId).has_value(), button.commandId.data());
            QVERIFY2(commands.insert(button.commandId).second, button.commandId.data());
            QVERIFY(!button.label.empty());
            const chrono_calc::Rectangle place = button.rectangle;
            QVERIFY(place.left >= 0 && place.top >= 0);
            QVERIFY(place.left + place.width <= chrono_calc::standard::clientWidth);
            QVERIFY(place.top + place.height <= chrono_calc::standard::clientHeight);
            QVERIFY(!overlap(place, chrono_calc::standard::displayFrame));
            for (std::size_t other = index + 1; other < buttons.size(); ++other) {
                QVERIFY2(!overlap(place, buttons[other].rectangle), button.commandId.data());
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
        QCOMPARE(built, 2);
        QCOMPARE(greyed, 19);
        const auto view = chrono_calc::standardMenus().front();
        QCOMPARE(view.items.front().entry.commandId, std::string_view("view.standard"));
        QVERIFY(view.items.front().entry.chosen);
    }
};

QTEST_GUILESS_MAIN(TestStandardView)
#include "test_standardview.moc"
