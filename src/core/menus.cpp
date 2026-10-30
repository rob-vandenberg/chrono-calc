#include "core/menus.h"

#include "core/view.h"

#include <string_view>
#include <utility>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/menus.cpp 0.5.12

// --- Version history ----------------------------------------------------
// v0.5.12: History (a tick, Ctrl+H), Digit grouping (a tick), Copy, Paste and the History submenu are built; the Edit
//          menu of each view (D-024)
// v0.4.11: View > Statistics is built
// v0.3.8: View > Programmer is built
// v0.2.7: View > Scientific is built
// v0.1.6: Initial: the menus View, Edit and Help of the Windows 7 Calculator (D-018)

namespace chrono_calc {

namespace {

// The dot groups of the View menu: the four views, and Basic with the three side panels
constexpr int viewGroup = 1;
constexpr int panelGroup = 2;

MenuEntry entry(std::string_view text, std::string_view shortcut = {}, std::string_view commandId = {})
{
    return MenuEntry{.text = text, .shortcut = shortcut, .commandId = commandId};
}

MenuItem item(std::string_view text, std::string_view shortcut = {}, std::string_view commandId = {})
{
    return MenuItem{.entry = entry(text, shortcut, commandId), .submenu = {}};
}

MenuItem dotted(MenuItem menuItem, int group, bool chosen)
{
    menuItem.entry.mark = MenuMark::Dot;
    menuItem.entry.dotGroup = group;
    menuItem.entry.chosen = chosen;
    return menuItem;
}

MenuItem afterSeparator(MenuItem menuItem)
{
    menuItem.entry.separatorBefore = true;
    return menuItem;
}

MenuItem submenu(std::string_view text, std::vector<MenuEntry> entries)
{
    return MenuItem{.entry = entry(text), .submenu = std::move(entries)};
}

MenuItem ticked(MenuItem menuItem)
{
    menuItem.entry.mark = MenuMark::Tick;
    return menuItem;
}

// An entry whose key the window handles itself
MenuEntry keyShown(MenuEntry menuEntry)
{
    menuEntry.shortcutShownOnly = true;
    return menuEntry;
}

MenuItem keyShown(MenuItem menuItem)
{
    menuItem.entry.shortcutShownOnly = true;
    return menuItem;
}

}  // namespace

// The four views, Basic, History, Digit grouping and the Edit menu are built; every other item has no command yet and
// shows greyed
std::vector<Menu> standardMenus()
{
    const Menu view{.title = "&View",
              .items = {
                  dotted(item("S&tandard", "Alt+1", "view.standard"), viewGroup, true),
                  dotted(item("&Scientific", "Alt+2", "view.scientific"), viewGroup, false),
                  dotted(item("&Programmer", "Alt+3", "view.programmer"), viewGroup, false),
                  dotted(item("St&atistics", "Alt+4", "view.statistics"), viewGroup, false),
                  afterSeparator(ticked(item("Histor&y", "Ctrl+H", "view.history"))),
                  ticked(item("D&igit grouping", {}, "view.digit_grouping")),
                  afterSeparator(dotted(item("&Basic", "Ctrl+F4", "view.basic"), panelGroup, true)),
                  dotted(item("&Unit conversion", "Ctrl+U"), panelGroup, false),
                  dotted(item("&Date calculation", "Ctrl+E"), panelGroup, false),
                  submenu("&Worksheets",
                          {entry("&Mortgage"), entry("&Vehicle lease"), entry("&Fuel economy (mpg)"),
                           entry("F&uel economy (L/100 km)")}),
              }};
    const Menu help{.title = "&Help",
              .items = {
                  item("&View Help", "F1"),
                  afterSeparator(item("&About Chrono Calc")),
              }};
    return {view, editMenu(View::Standard), help};
}

Menu editMenu(View view)
{
    Menu edit{.title = "&Edit", .items = {item("&Copy", "Ctrl+C", "edit.copy"), item("&Paste", "Ctrl+V", "edit.paste")}};
    switch (view) {
    case View::Standard:
    case View::Scientific:
        edit.items.push_back(afterSeparator(submenu(
            "&History", {entry("Copy h&istory", {}, "history.copy"), keyShown(entry("&Edit", "F2", "history.edit")),
                         keyShown(entry("Ca&ncel edit", "Esc", "history.cancel_edit")),
                         entry("C&lear", "Ctrl+Shift+D", "history.clear")})));
        break;
    case View::Programmer:
        // Windows 7 leaves the submenu out here; it is shown greyed, as everything unusable is
        edit.items.push_back(afterSeparator(submenu(
            "&History", {entry("Copy h&istory"), entry("&Edit", "F2"), entry("Ca&ncel edit", "Esc"), entry("C&lear", "Ctrl+Shift+D")})));
        break;
    case View::Statistics:
        edit.items.push_back(afterSeparator(item("Copy &dataset", {}, "statistics.copy_dataset")));
        edit.items.push_back(keyShown(item("&Edit", "F2", "statistics.edit")));
        edit.items.push_back(keyShown(item("Ca&ncel edit", "Esc", "statistics.cancel_edit")));
        edit.items.push_back(keyShown(item("C&lear", "D", "statistics.clear_all")));
        break;
    }
    return edit;
}

}  // namespace chrono_calc
