#pragma once

#include "core/enginetexts.h"

#include "CalcManager/CalculatorManager.h"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/calculator.h 0.0.4

// --- Version history ----------------------------------------------------
// v0.0.4: Initial: the engine wrapper; runs commands and keeps what the engine shows (D-015)

namespace chrono_calc {

// One calculator: Microsoft's engine behind a plain interface. Commands go in by id (commands.h); what the engine
// reports (the number, the expression line, an error, the memory) is kept here as plain values for the window.
class Calculator
{
public:
    Calculator();
    ~Calculator();
    Calculator(const Calculator&) = delete;
    Calculator& operator=(const Calculator&) = delete;
    Calculator(Calculator&&) = delete;
    Calculator& operator=(Calculator&&) = delete;

    // Runs the command with this id; false when no command has that id (nothing happens then)
    bool run(std::string_view commandId);

    // The number the display shows, or the error text while in error
    [[nodiscard]] std::wstring display() const;
    // The expression line above the number, for example "2 + 3 -"
    [[nodiscard]] std::wstring expression() const;
    [[nodiscard]] bool isError() const;
    // Whether a number is in memory (the display shows "M" then)
    [[nodiscard]] bool hasMemory() const;
    // How many memory places the engine holds: always 0 or 1, because Windows 7 has one memory
    [[nodiscard]] std::size_t memoryPlaces() const;

private:
    class Display;

    EngineTexts m_texts;
    std::unique_ptr<Display> m_display;
    std::unique_ptr<CalculationManager::CalculatorManager> m_manager;
};

}  // namespace chrono_calc
