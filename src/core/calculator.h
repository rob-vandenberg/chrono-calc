#pragma once

#include "core/enginetexts.h"
#include "core/view.h"

#include "CalcManager/CalculatorHistory.h"
#include "Header Files/CalcEngine.h"
#include "Header Files/Rational.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/calculator.h 0.3.8

// --- Version history ----------------------------------------------------
// v0.3.8: The Programmer view: a third engine in integer mode (64 bits); bases and word sizes kept over a view change;
//         the bits; which commands each view and base allows (D-021)
// v0.2.7: The Scientific view: its own engine with precedence and 32 digits, the one memory shared; Inv, the angle
//         units, F-E, brackets; the expression line from the engine as Windows 7 shows it (D-020)
// v0.1.6: The last result, for a percent right after = (Windows 7)
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

    [[nodiscard]] View view() const;
    // The Scientific view's switches: Inv, the angle unit, F-E (numbers shown as 1,2e+3)
    [[nodiscard]] bool isInverse() const;
    [[nodiscard]] AngleUnit angleUnit() const;
    [[nodiscard]] bool isScientificNotation() const;
    // How many brackets are open (the Scientific view shows "(=1")
    [[nodiscard]] unsigned int openBrackets() const;
    // The Programmer view's number base and word size, and how many bits that word has
    [[nodiscard]] NumberBase numberBase() const;
    [[nodiscard]] WordSize wordSize() const;
    [[nodiscard]] int wordBits() const;
    // The 64 bits of the current value, bit 63 first, as the engine writes it in binary ('0' or '1'); all '0' outside
    // the Programmer view and while an error shows
    [[nodiscard]] std::string bits();
    // Whether a choice (an angle unit, a base, a word size) is the chosen one
    [[nodiscard]] bool isChosen(std::string_view commandId) const;
    // Whether the command does something in a view and base (Windows 7 greys its button otherwise)
    [[nodiscard]] static bool isAvailable(View view, NumberBase base, std::string_view commandId);
    [[nodiscard]] bool isAvailable(std::string_view commandId) const;

private:
    class Display;

    void switchView(View view);
    void resetWrapperState();
    void runScientific(std::uintptr_t opCode);
    void runProgrammer(std::uintptr_t opCode);
    [[nodiscard]] CCalcEngine* engineOf(View view) const;
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
    std::shared_ptr<CalculationManager::CalculatorHistory> m_standardHistory;
    std::shared_ptr<CalculationManager::CalculatorHistory> m_scientificHistory;
    std::shared_ptr<CalculationManager::CalculatorHistory> m_programmerHistory;
    // One engine per view, as Windows 10 has: Standard calculates from left to right with 16 digits, Scientific with
    // precedence and 32 digits, Programmer with precedence on 64-bit integers. m_engine is the engine of the current view.
    std::unique_ptr<CCalcEngine> m_standardEngine;
    std::unique_ptr<CCalcEngine> m_scientificEngine;
    std::unique_ptr<CCalcEngine> m_programmerEngine;
    CCalcEngine* m_engine = nullptr;
    View m_view = View::Standard;
    bool m_inverse = false;
    AngleUnit m_angleUnit = AngleUnit::Degrees;
    bool m_scientificNotation = false;
    NumberBase m_numberBase = NumberBase::Decimal;
    WordSize m_wordSize = WordSize::Qword;

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
    // The result of the last =, while no operator came after it: a percent then takes it as its base
    std::optional<CalcEngine::Rational> m_result;
};

}  // namespace chrono_calc
