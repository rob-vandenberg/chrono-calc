#include "core/commands.h"

#include "core/statistics.h"

#include "CalcManager/Command.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/commands.cpp 0.4.11

// --- Version history ----------------------------------------------------
// v0.4.11: The commands of the Statistics view (Add, CAD, the statistics, the list's selection and Delete)
// v0.3.8: The commands of the Programmer view (bases, word sizes, A to F, logic, shifts, rotations, the 64 bits)
// v0.2.7: The commands of the Scientific view (functions, brackets, Inv, the angle units, F-E, Exp, Mod)
// v0.1.6: view.basic (View > Basic: no side panel)
// v0.1.5: The memory commands are the engine's own (its one memory register, as in Windows 7)
// v0.0.4: Initial: the command registry with the commands of the Standard view (D-015)

namespace chrono_calc {

namespace {

using CalculationManager::Command;

struct Entry
{
    std::string_view id;
    CommandTarget target;
};

constexpr Entry engine(std::string_view id, Command command)
{
    return {.id = id, .target = {.kind = CommandKind::Engine, .engineCommand = command, .statistic = Statistic::Mean}};
}

constexpr Entry own(std::string_view id, CommandKind kind)
{
    return {.id = id, .target = {.kind = kind, .engineCommand = Command::CommandNULL, .statistic = Statistic::Mean}};
}

constexpr Entry statistic(std::string_view id, Statistic which)
{
    return {.id = id,
            .target = {.kind = CommandKind::StatisticsFunction, .engineCommand = Command::CommandNULL, .statistic = which}};
}

constexpr std::array registry = {
    engine("digit.0", Command::Command0),
    engine("digit.1", Command::Command1),
    engine("digit.2", Command::Command2),
    engine("digit.3", Command::Command3),
    engine("digit.4", Command::Command4),
    engine("digit.5", Command::Command5),
    engine("digit.6", Command::Command6),
    engine("digit.7", Command::Command7),
    engine("digit.8", Command::Command8),
    engine("digit.9", Command::Command9),
    engine("decimal", Command::CommandPNT),
    engine("add", Command::CommandADD),
    engine("subtract", Command::CommandSUB),
    engine("multiply", Command::CommandMUL),
    engine("divide", Command::CommandDIV),
    engine("equals", Command::CommandEQU),
    engine("clear", Command::CommandCLEAR),
    engine("clear_entry", Command::CommandCENTR),
    engine("backspace", Command::CommandBACK),
    engine("negate", Command::CommandSIGN),
    engine("sqrt", Command::CommandSQRT),
    engine("percent", Command::CommandPERCENT),
    engine("reciprocal", Command::CommandREC),
    engine("memory.clear", Command::CommandMCLEAR),
    engine("memory.recall", Command::CommandRECALL),
    engine("memory.store", Command::CommandSTORE),
    engine("memory.add", Command::CommandMPLUS),
    engine("memory.subtract", Command::CommandMMINUS),
    // The Scientific view
    engine("sin", Command::CommandSIN),
    engine("cos", Command::CommandCOS),
    engine("tan", Command::CommandTAN),
    engine("sinh", Command::CommandSINH),
    engine("cosh", Command::CommandCOSH),
    engine("tanh", Command::CommandTANH),
    engine("ln", Command::CommandLN),
    engine("log", Command::CommandLOG),
    engine("power_of_ten", Command::CommandPOW10),
    engine("square", Command::CommandSQR),
    engine("cube", Command::CommandCUB),
    engine("cube_root", Command::CommandCUBEROOT),
    engine("power", Command::CommandPWR),
    engine("root", Command::CommandROOT),
    engine("factorial", Command::CommandFAC),
    engine("integer", Command::CommandCHOP),
    engine("dms", Command::CommandDMS),
    engine("pi", Command::CommandPI),
    engine("scientific_notation", Command::CommandFE),
    engine("exponent", Command::CommandEXP),
    engine("modulo", Command::CommandMOD),
    engine("open_bracket", Command::CommandOPENP),
    engine("close_bracket", Command::CommandCLOSEP),
    engine("angle.degrees", Command::CommandDEG),
    engine("angle.radians", Command::CommandRAD),
    engine("angle.grads", Command::CommandGRAD),
    // The Programmer view
    engine("digit.a", Command::CommandA),
    engine("digit.b", Command::CommandB),
    engine("digit.c", Command::CommandC),
    engine("digit.d", Command::CommandD),
    engine("digit.e", Command::CommandE),
    engine("digit.f", Command::CommandF),
    engine("and", Command::CommandAnd),
    engine("or", Command::CommandOR),
    engine("xor", Command::CommandXor),
    engine("not", Command::CommandNot),
    engine("shift_left", Command::CommandLSHF),
    engine("shift_right", Command::CommandRSHF),
    engine("rotate_left", Command::CommandROL),
    engine("rotate_right", Command::CommandROR),
    engine("base.hexadecimal", Command::CommandHex),
    engine("base.decimal", Command::CommandDec),
    engine("base.octal", Command::CommandOct),
    engine("base.binary", Command::CommandBin),
    engine("word.qword", Command::CommandQword),
    engine("word.dword", Command::CommandDword),
    engine("word.word", Command::CommandWord),
    engine("word.byte", Command::CommandByte),
    // The 64 bits of the bit display: a click toggles one (the engine's commands 700 to 763)
    engine("bit.0", static_cast<Command>(700)),
    engine("bit.1", static_cast<Command>(701)),
    engine("bit.2", static_cast<Command>(702)),
    engine("bit.3", static_cast<Command>(703)),
    engine("bit.4", static_cast<Command>(704)),
    engine("bit.5", static_cast<Command>(705)),
    engine("bit.6", static_cast<Command>(706)),
    engine("bit.7", static_cast<Command>(707)),
    engine("bit.8", static_cast<Command>(708)),
    engine("bit.9", static_cast<Command>(709)),
    engine("bit.10", static_cast<Command>(710)),
    engine("bit.11", static_cast<Command>(711)),
    engine("bit.12", static_cast<Command>(712)),
    engine("bit.13", static_cast<Command>(713)),
    engine("bit.14", static_cast<Command>(714)),
    engine("bit.15", static_cast<Command>(715)),
    engine("bit.16", static_cast<Command>(716)),
    engine("bit.17", static_cast<Command>(717)),
    engine("bit.18", static_cast<Command>(718)),
    engine("bit.19", static_cast<Command>(719)),
    engine("bit.20", static_cast<Command>(720)),
    engine("bit.21", static_cast<Command>(721)),
    engine("bit.22", static_cast<Command>(722)),
    engine("bit.23", static_cast<Command>(723)),
    engine("bit.24", static_cast<Command>(724)),
    engine("bit.25", static_cast<Command>(725)),
    engine("bit.26", static_cast<Command>(726)),
    engine("bit.27", static_cast<Command>(727)),
    engine("bit.28", static_cast<Command>(728)),
    engine("bit.29", static_cast<Command>(729)),
    engine("bit.30", static_cast<Command>(730)),
    engine("bit.31", static_cast<Command>(731)),
    engine("bit.32", static_cast<Command>(732)),
    engine("bit.33", static_cast<Command>(733)),
    engine("bit.34", static_cast<Command>(734)),
    engine("bit.35", static_cast<Command>(735)),
    engine("bit.36", static_cast<Command>(736)),
    engine("bit.37", static_cast<Command>(737)),
    engine("bit.38", static_cast<Command>(738)),
    engine("bit.39", static_cast<Command>(739)),
    engine("bit.40", static_cast<Command>(740)),
    engine("bit.41", static_cast<Command>(741)),
    engine("bit.42", static_cast<Command>(742)),
    engine("bit.43", static_cast<Command>(743)),
    engine("bit.44", static_cast<Command>(744)),
    engine("bit.45", static_cast<Command>(745)),
    engine("bit.46", static_cast<Command>(746)),
    engine("bit.47", static_cast<Command>(747)),
    engine("bit.48", static_cast<Command>(748)),
    engine("bit.49", static_cast<Command>(749)),
    engine("bit.50", static_cast<Command>(750)),
    engine("bit.51", static_cast<Command>(751)),
    engine("bit.52", static_cast<Command>(752)),
    engine("bit.53", static_cast<Command>(753)),
    engine("bit.54", static_cast<Command>(754)),
    engine("bit.55", static_cast<Command>(755)),
    engine("bit.56", static_cast<Command>(756)),
    engine("bit.57", static_cast<Command>(757)),
    engine("bit.58", static_cast<Command>(758)),
    engine("bit.59", static_cast<Command>(759)),
    engine("bit.60", static_cast<Command>(760)),
    engine("bit.61", static_cast<Command>(761)),
    engine("bit.62", static_cast<Command>(762)),
    engine("bit.63", static_cast<Command>(763)),
    own("inverse", CommandKind::Inverse),
    own("view.standard", CommandKind::ViewStandard),
    own("view.scientific", CommandKind::ViewScientific),
    own("view.programmer", CommandKind::ViewProgrammer),
    own("view.statistics", CommandKind::ViewStatistics),
    own("view.basic", CommandKind::ViewBasic),
    // The Statistics view
    own("statistics.add", CommandKind::StatisticsAdd),
    own("statistics.clear_all", CommandKind::StatisticsClearAll),
    statistic("statistics.mean", Statistic::Mean),
    statistic("statistics.mean_of_squares", Statistic::MeanOfSquares),
    statistic("statistics.sum", Statistic::Sum),
    statistic("statistics.sum_of_squares", Statistic::SumOfSquares),
    statistic("statistics.deviation", Statistic::PopulationDeviation),
    statistic("statistics.sample_deviation", Statistic::SampleDeviation),
    own("statistics.select_previous", CommandKind::StatisticsSelectPrevious),
    own("statistics.select_next", CommandKind::StatisticsSelectNext),
    own("statistics.delete", CommandKind::StatisticsDelete),
};

// "statistics.select.N": N is the item's index, at most nine digits
std::optional<CommandTarget> selectCommand(std::string_view id)
{
    constexpr std::string_view prefix = "statistics.select.";
    constexpr std::size_t mostDigits = 9;
    if (!id.starts_with(prefix)) {
        return std::nullopt;
    }
    const std::string_view digits = id.substr(prefix.size());
    if (digits.empty() || digits.size() > mostDigits || !std::ranges::all_of(digits, [](char digit) { return digit >= '0' && digit <= '9'; })) {
        return std::nullopt;
    }
    std::size_t index = 0;
    for (const char digit : digits) {
        index = (index * 10) + static_cast<std::size_t>(digit - '0');
    }
    return CommandTarget{.kind = CommandKind::StatisticsSelect, .engineCommand = Command::CommandNULL, .statistic = Statistic::Mean, .index = index};
}

}  // namespace

std::optional<CommandTarget> findCommand(std::string_view id)
{
    if (auto select = selectCommand(id)) {
        return select;
    }
    const auto* found = std::ranges::find(registry, id, &Entry::id);
    if (found == registry.end()) {
        return std::nullopt;
    }
    return found->target;
}

std::vector<std::string_view> commandIds()
{
    std::vector<std::string_view> ids;
    ids.reserve(registry.size());
    for (const Entry& entry : registry) {
        ids.push_back(entry.id);
    }
    return ids;
}

}  // namespace chrono_calc
