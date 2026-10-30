#pragma once

#include <cstdint>
#include "core/view.h"

#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/menus.h 0.5.12

// --- Version history ----------------------------------------------------
// v0.5.12: History, Digit grouping, Copy, Paste and the History submenu are built; the Edit menu of each view
// v0.1.6: Initial: the menus View, Edit and Help of the Windows 7 Calculator (D-018)

namespace chrono_calc {

// How a menu item looks: a plain item, one of a group with a dot (one chosen at a time), or one with a tick
enum class MenuMark : std::uint8_t
{
    None,
    Dot,
    Tick,
};

// One menu entry. An entry without a command is not built yet: the window shows it greyed, never hides it.
// The text marks its access key with & ("&Standard"); the shortcut is written as the window shows it ("Alt+1").
struct MenuEntry
{
    std::string_view text;
    std::string_view shortcut;
    std::string_view commandId;
    MenuMark mark = MenuMark::None;
    // The items with a dot that belong together (only one of them is chosen); 0 for none
    int dotGroup = 0;
    // Whether the item is the chosen one of its dot group, or ticked
    bool chosen = false;
    bool separatorBefore = false;
    // The shortcut is only shown: the window's keys do it, because the key means something else elsewhere (F2, Esc, D)
    bool shortcutShownOnly = false;
};

// One item of a menu: an entry, or a submenu of entries (the entry gives its text then)
struct MenuItem
{
    MenuEntry entry;
    std::vector<MenuEntry> submenu;
};

struct Menu
{
    std::string_view title;
    std::vector<MenuItem> items;
};

// The menus as in the Windows 7 Calculator (tests/reference/windows7-standard.md, part B), in the Standard view
std::vector<Menu> standardMenus();

// The Edit menu of a view (windows7-history.md parts C and G): Copy and Paste; in Standard and Scientific the History
// submenu (greyed in Programmer, where Windows 7 leaves it out); in Statistics Copy dataset, Edit, Cancel edit, Clear
Menu editMenu(View view);

}  // namespace chrono_calc
