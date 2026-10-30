#include "core/menus.h"

#include <string_view>
#include <utility>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/menus.cpp 0.4.11

// --- Version history ----------------------------------------------------
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

}  // namespace

// The four views and Basic are built; every other item has no command yet and shows greyed
std::vector<Menu> standardMenus()
{
    const Menu view{.title = "&View",
              .items = {
                  dotted(item("S&tandard", "Alt+1", "view.standard"), viewGroup, true),
                  dotted(item("&Scientific", "Alt+2", "view.scientific"), viewGroup, false),
                  dotted(item("&Programmer", "Alt+3", "view.programmer"), viewGroup, false),
                  dotted(item("St&atistics", "Alt+4", "view.statistics"), viewGroup, false),
                  afterSeparator(item("Histor&y", "Ctrl+H")),
                  item("D&igit grouping"),
                  afterSeparator(dotted(item("&Basic", "Ctrl+F4", "view.basic"), panelGroup, true)),
                  dotted(item("&Unit conversion", "Ctrl+U"), panelGroup, false),
                  dotted(item("&Date calculation", "Ctrl+E"), panelGroup, false),
                  submenu("&Worksheets",
                          {entry("&Mortgage"), entry("&Vehicle lease"), entry("&Fuel economy (mpg)"),
                           entry("F&uel economy (L/100 km)")}),
              }};
    const Menu edit{.title = "&Edit",
              .items = {
                  item("&Copy", "Ctrl+C"),
                  item("&Paste", "Ctrl+V"),
                  afterSeparator(submenu("&History", {entry("Copy h&istory"), entry("&Edit", "F2"),
                                                      entry("Ca&ncel edit", "Esc"), entry("C&lear", "Ctrl+Shift+D")})),
              }};
    const Menu help{.title = "&Help",
              .items = {
                  item("&View Help", "F1"),
                  afterSeparator(item("&About Chrono Calc")),
              }};
    return {view, edit, help};
}

}  // namespace chrono_calc
