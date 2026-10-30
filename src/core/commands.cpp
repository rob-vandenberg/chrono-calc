#include "core/commands.h"

#include "CalcManager/Command.h"

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/commands.cpp 0.1.6

// --- Version history ----------------------------------------------------
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
    return {.id = id, .target = {.kind = CommandKind::Engine, .engineCommand = command}};
}

constexpr Entry own(std::string_view id, CommandKind kind)
{
    return {.id = id, .target = {.kind = kind, .engineCommand = Command::CommandNULL}};
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
    own("view.standard", CommandKind::ViewStandard),
    own("view.basic", CommandKind::ViewBasic),
};

}  // namespace

std::optional<CommandTarget> findCommand(std::string_view id)
{
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
