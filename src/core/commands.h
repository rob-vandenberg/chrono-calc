#pragma once

#include "core/statistics.h"
#include "core/view.h"

#include "CalcManager/Command.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/commands.h 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: The side panels (View > Basic, Unit conversion, Date calculation, Worksheets), the Unit conversion and Date
//          calculation panels' and the Worksheets' commands
// v0.5.12: History, Digit grouping, Copy and Paste: their commands; a command may take a text (Paste, Recalculate)
// v0.4.11: The commands of the Statistics view (Add, CAD, the statistics, the list's selection and Delete)
// v0.3.8: The commands of the Programmer view (bases, word sizes, A to F, logic, shifts, rotations, the 64 bits)
// v0.2.7: The commands of the Scientific view (functions, brackets, Inv, the angle units, F-E, Exp, Mod)
// v0.1.6: view.basic (View > Basic: no side panel)
// v0.1.5: The memory commands are the engine's own (its one memory register, as in Windows 7)
// v0.0.4: Initial: the command registry with the commands of the Standard view (D-015)

namespace chrono_calc {

// What a command does: one engine command, or a view change of the calculator itself
enum class CommandKind : std::uint8_t
{
    Engine,
    ViewStandard,
    ViewScientific,
    ViewProgrammer,
    ViewStatistics,
    // View > Basic, Unit conversion, Date calculation, Worksheets: the panel beside the calculator (one at a time)
    ViewSidePanel,
    // Inv: the next function that has an inverse form gives that form (Windows 7's Inv button)
    Inverse,
    // The Statistics view: Add (the number to the list), CAD (clear all data), a statistic of the list, and the list's
    // selection (the item above or below the selected one, Delete of the selected one)
    StatisticsAdd,
    StatisticsClearAll,
    StatisticsFunction,
    StatisticsSelect,
    StatisticsSelectPrevious,
    StatisticsSelectNext,
    StatisticsDelete,
    // Copy dataset: every value of the list as text; Edit (F2) of the selected value, its Cancel, and the edited text
    // put in place of the value
    StatisticsCopyDataset,
    StatisticsEdit,
    StatisticsCancelEdit,
    StatisticsApplyEdit,
    // View > History (shows or hides the list of calculations) and View > Digit grouping, each turned on and off
    ViewHistory,
    ViewDigitGrouping,
    // Edit > Copy (the number shown, as text) and Paste (a text, read as typed keys)
    Copy,
    Paste,
    // The history of the view: an entry chosen (its result shown), the entry above or below, Copy history, Clear, and
    // Edit: an entry opened for editing, the edit dropped, or the edited text calculated in place of the entry
    HistorySelect,
    HistorySelectPrevious,
    HistorySelectNext,
    HistoryCopy,
    HistoryClear,
    HistoryEdit,
    HistoryCancelEdit,
    HistoryRecalculate,
    // The Unit conversion panel: a unit type chosen, the From or To unit chosen, the From box's text
    UnitType,
    UnitFrom,
    UnitTo,
    UnitInput,
    // The Date calculation panel: a mode chosen, a date set (the Difference From and To, the Add or subtract From), Add
    // or Subtract chosen, a number box's text, its arrows, and Calculate
    DateMode,
    DateDifferenceFrom,
    DateDifferenceTo,
    DateShiftFrom,
    DateAdd,
    DateSubtract,
    DateField,
    DateFieldUp,
    DateFieldDown,
    DateCalculate,
    // A Worksheet (the one shown): the value it calculates chosen from its list, a box's text, and Calculate
    WorksheetChoose,
    WorksheetValue,
    WorksheetCalculate,
};

struct CommandTarget
{
    CommandKind kind = CommandKind::Engine;
    CalculationManager::Command engineCommand = CalculationManager::Command::CommandNULL;
    // The statistic of a StatisticsFunction command
    Statistic statistic = Statistic::Mean;
    // The list item of a StatisticsSelect, HistorySelect, UnitType, UnitFrom, UnitTo, DateMode or WorksheetChoose
    // command; the number box of a DateField, DateFieldUp or DateFieldDown command (0 years, 1 months, 2 days); the
    // value of a WorksheetValue command (its place in the worksheet's own order of values)
    std::size_t index = 0;
    // The panel of a ViewSidePanel command
    SidePanel panel = SidePanel::Basic;
};

// Every user action is a command with an id (AGENTS.md 4.3), for example "digit.7", "add", "memory.store"; some take
// their parameter in the id: "statistics.select.N" selects item N (from 0) of the Statistics view's list,
// "history.select.N" entry N of the history, "unit.type.N", "unit.from.N" and "unit.to.N" item N of the Unit conversion
// panel's lists, "date.mode.N" the Date calculation panel's mode, "worksheet.choose.N" item N of a Worksheet's list,
// "worksheet.value.N" value N of a Worksheet; "edit.paste", "history.recalculate", "statistics.apply_edit",
// "unit.input", the dates ("date.difference_from=2026-10-30"), the number boxes ("date.years=5") and the Worksheet values
// ("worksheet.value.2=30") take a text (Calculator::run)
std::optional<CommandTarget> findCommand(std::string_view id);

// Every command id, in the order of the registry (without the items of the commands that carry an index in their id)
std::vector<std::string_view> commandIds();

}  // namespace chrono_calc
