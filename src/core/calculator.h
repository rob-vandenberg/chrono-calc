#pragma once

#include "core/enginetexts.h"

#include "CalcManager/CalculatorHistory.h"
#include "Header Files/CalcEngine.h"
#include "Header Files/Rational.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/calculator.h 0.1.5

// --- Version history ----------------------------------------------------
// v0.1.5: Drives the engine directly (its one memory register is Windows 7's memory); Windows 7's expression line and
//         percent; the separators of the regional settings (D-017)
// v0.0.4: Initial: the engine wrapper; runs commands and keeps what the engine shows (D-015)

namespace chrono_calc {

// One calculator: Microsoft's engine behind a plain interface. Commands go in by id (commands.h); what the engine
// reports (the number, an error) and the expression line built as Windows 7 shows it are kept as plain values.
class Calculator
{
public:
    explicit Calculator(Separators separators = {});
    ~Calculator();
    Calculator(const Calculator&) = delete;
    Calculator& operator=(const Calculator&) = delete;
    Calculator(Calculator&&) = delete;
    Calculator& operator=(Calculator&&) = delete;

    // Runs the command with this id; false when no command has that id (nothing happens then)
    bool run(std::string_view commandId);

    // The number the display shows, or the error text while in error
    [[nodiscard]] std::wstring display() const;
    // The expression line above the number, as Windows 7 shows it, for example "2 + 3 *" or "2 + sqrt(9)"
    [[nodiscard]] std::wstring expression() const;
    [[nodiscard]] bool isError() const;
    // Whether a number other than 0 is in the one memory (the display shows "M" then)
    [[nodiscard]] bool hasMemory();

private:
    class Display;

    void setStandardView();
    void runEngine(std::uintptr_t opCode);
    void binaryOperator(std::uintptr_t opCode);
    void percent();
    // The engine's current value, read exactly through its memory register; the memory is left as it was
    [[nodiscard]] CalcEngine::Rational currentValue();
    // Makes the value the engine's current value, as a memory recall does; the memory is left as it was
    void enterValue(const CalcEngine::Rational& value);
    // The engine's memory, put back after reading (reading takes it out of the engine)
    [[nodiscard]] CalcEngine::Rational memoryValue();

    EngineTexts m_texts;
    std::unique_ptr<Display> m_display;
    std::shared_ptr<CalculationManager::CalculatorHistory> m_history;
    std::unique_ptr<CCalcEngine> m_engine;

    // The expression line (Windows 7): the finished part ("2 + 3 *"), whether the last command was a binary operator,
    // the operator pending in the engine, and its left value while it is multiply or divide (for percent)
    std::wstring m_chain;
    bool m_lastWasOperator = false;
    std::uintptr_t m_pendingOperator = 0;
    std::optional<CalcEngine::Rational> m_leftValue;
    // What the expression line shows for the current value after a Windows 7 percent ("100 * 5")
    std::wstring m_percentText;
    // After = the engine's own last part ("4=") is not shown: Windows 7 empties the line
    bool m_afterEquals = false;
};

}  // namespace chrono_calc
