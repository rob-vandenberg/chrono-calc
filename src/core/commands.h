#pragma once

#include "CalcManager/Command.h"

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/commands.h 0.0.4

// --- Version history ----------------------------------------------------
// v0.0.4: Initial: the command registry with the commands of the Standard view (D-015)

namespace chrono_calc {

// What a command does: one engine command, or a memory or view action of the calculator itself
enum class CommandKind : std::uint8_t
{
    Engine,
    MemoryClear,
    MemoryRecall,
    MemoryStore,
    MemoryAdd,
    MemorySubtract,
    ViewStandard,
};

struct CommandTarget
{
    CommandKind kind = CommandKind::Engine;
    CalculationManager::Command engineCommand = CalculationManager::Command::CommandNULL;
};

// Every user action is a command with an id (AGENTS.md 4.3), for example "digit.7", "add", "memory.store"
std::optional<CommandTarget> findCommand(std::string_view id);

// Every command id, in the order of the registry
std::vector<std::string_view> commandIds();

}  // namespace chrono_calc
