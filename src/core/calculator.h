#pragma once

#include "core/commands.h"
#include "core/enginetexts.h"
#include "core/historyedit.h"
#include "core/view.h"

#include "CalcManager/CalculatorHistory.h"
#include "Header Files/CalcEngine.h"
#include "Header Files/Rational.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/calculator.h 0.5.12

// --- Version history ----------------------------------------------------
// v0.5.12: History (one per view, from the engine; choosing an entry, Copy history, Clear, Edit), Digit grouping,
//          Copy and Paste (D-024)
// v0.4.11: The Statistics view: a fourth engine (16 digits, as Windows 7), the list of values with its selection, Add,
//          CAD, the statistics computed on the engine's exact fractions (D-010, D-023)
// v0.3.8: The Programmer view: a third engine in integer mode (64 bits); bases and word sizes kept over a view change;
//         the bits; which commands each view and base allows (D-021)
// v0.2.7: The Scientific view: its own engine with precedence and 32 digits, the one memory shared; Inv, the angle
//         units, F-E, brackets; the expression line from the engine as Windows 7 shows it (D-020)
// v0.1.6: The last result, for a percent right after = (Windows 7)
// v0.1.5: Drives the engine directly (its one memory register is Windows 7's memory); Windows 7's expression line and
//         percent; the separators of the regional settings (D-017)
// v0.0.4: Initial: the engine wrapper; runs commands and keeps what the engine shows (D-015)

namespace chrono_calc {

// One calculation of the history: the expression as the engine writes it ("2 + 3") and its result as the display wrote
// it, without Digit grouping ("0.3333333333333333")
struct HistoryEntry
{
    std::wstring expression;
    std::wstring result;
};

// One calculator: Microsoft's engine behind a plain interface. Commands go in by id (commands.h); what the engine
// reports (the number, an error) and the expression line built as Windows 7 shows it are kept as plain values.
class Calculator
{
public:
    explicit Calculator(const Separators& separators = {});
    ~Calculator();
    Calculator(const Calculator&) = delete;
    Calculator& operator=(const Calculator&) = delete;
    Calculator(Calculator&&) = delete;
    Calculator& operator=(Calculator&&) = delete;

    // Runs the command with this id; false when no command has that id (nothing happens then)
    bool run(std::string_view commandId);
    // Runs a command that takes a text: "edit.paste" reads the text as typed keys (Windows 7), "history.recalculate"
    // calculates the edited entry; every other command ignores the text
    bool run(std::string_view commandId, std::wstring_view text);

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

    // The Statistics view's list: every value Add put in, the oldest first, each written as the display writes a
    // number (F-E included); the list is kept over a view change, as in Windows 7
    [[nodiscard]] std::vector<std::wstring> statisticsValues();
    [[nodiscard]] std::size_t statisticsCount() const;
    // The selected item of the list (Add selects the new one); nothing when the list is empty
    // (the command "statistics.select.N" selects item N, as a click on it does)
    [[nodiscard]] std::optional<std::size_t> selectedValue() const;

    // View > Digit grouping (Windows 7 starts with it off)
    [[nodiscard]] bool isDigitGrouping() const;
    // View > History: shown or hidden, one setting for the Standard and Scientific views (the only ones with a history)
    [[nodiscard]] bool isHistoryShown() const;
    // The current view's history, each calculation as Windows 7 writes it ("2 + 3"), the oldest first; empty in the
    // Programmer and Statistics views. Choosing an entry ("history.select.N") shows its result.
    [[nodiscard]] std::vector<std::wstring> historyEntries() const;
    [[nodiscard]] std::optional<std::size_t> selectedHistoryEntry() const;
    // The entry being edited (Edit > History > Edit, F2); nothing while none is
    [[nodiscard]] std::optional<std::size_t> editedHistoryEntry() const;
    // The value being edited in the Statistics view (Edit > Edit, F2); nothing while none is
    [[nodiscard]] std::optional<std::size_t> editedStatisticsValue() const;
    // The text the last copy command gave for the clipboard (Copy, Copy history, Copy dataset), given once; nothing
    // when the command leaves the clipboard as it is (Copy history of an empty history). Lines end with "\n".
    [[nodiscard]] std::optional<std::wstring> takeCopiedText();

private:
    class Display;

    void switchView(View view);
    void resetWrapperState();
    void runScientific(std::uintptr_t opCode);
    void runProgrammer(std::uintptr_t opCode);
    void runStatistics(std::uintptr_t opCode);
    void runStatisticsCommand(const CommandTarget& target);
    void showStatistic(Statistic statistic);
    void deleteSelectedValue();
    [[nodiscard]] CCalcEngine* engineOf(View view) const;
    void runEngine(std::uintptr_t opCode);
    void setDigitGrouping(bool grouping);
    void runTarget(const CommandTarget& target, std::wstring_view text);
    [[nodiscard]] bool runOwnCommand(const CommandTarget& target, std::wstring_view text);
    void runEditCommand(const CommandTarget& target, std::wstring_view text);
    void runEngineCommand(const CommandTarget& target);
    void runStandard(std::uintptr_t opCode);
    [[nodiscard]] static bool isAvailable(View view, NumberBase base, const CommandTarget& target);
    void runHistoryCommand(const CommandTarget& target, std::wstring_view text);
    // The commands a pasted text gives, as its characters typed one after the other (Windows 7)
    [[nodiscard]] std::vector<std::string_view> pasteCommands(std::wstring_view text) const;
    std::size_t pasteCharacter(std::wstring_view text, std::vector<std::string_view>& commands, bool& startsSign) const;
    std::size_t pasteKey(std::wstring_view text, std::vector<std::string_view>& commands, bool& startsSign) const;
    [[nodiscard]] static std::optional<std::string_view> pastedMemoryCode(wchar_t letter);
    [[nodiscard]] std::optional<std::string_view> pastedCommand(wchar_t character) const;
    [[nodiscard]] std::optional<std::string_view> keyOfPastedCharacter(wchar_t character) const;
    [[nodiscard]] bool startsNumber(std::wstring_view text) const;
    // The history of a view: nothing for the views without one (Programmer, Statistics)
    [[nodiscard]] std::vector<HistoryEntry>* historyOf(View view);
    [[nodiscard]] const std::vector<HistoryEntry>* historyOf(View view) const;
    [[nodiscard]] std::optional<std::size_t>& historySelection();
    [[nodiscard]] const std::optional<std::size_t>& historySelection() const;
    // The engine put a calculation in its history: it becomes the newest entry, selected (Windows 7); a step of a
    // Standard chain (at its next operator) only extends the chain's entry
    void addHistoryEntry(bool chainStep);
    // An entry becomes the selected one and its result the current value, as a memory recall does; an error shown is
    // cleared first (Windows 7)
    void chooseHistoryEntry(std::size_t index);
    // Edit > History > Edit: the edited text calculated in place of the entry; an edit that cannot be read or ends in an
    // error changes nothing and stays open
    void recalculateHistoryEntry(std::wstring_view text);
    [[nodiscard]] std::optional<std::wstring> typeSteps(CCalcEngine& engine, const std::vector<EditStep>& steps);
    // The Statistics view's edit of the selected value: a number replaces it, an empty text removes it
    void applyStatisticsEdit(std::wstring_view text);
    [[nodiscard]] NumberRules numberRules() const;
    // The evaluation engine of the current view, cleared
    [[nodiscard]] CCalcEngine& evaluationEngine();
    static void endEvaluation(CCalcEngine& engine);
    [[nodiscard]] static bool closeBrackets(CCalcEngine& engine, const Display& display);
    // Types the number into an engine, as its keys would
    static void typeNumber(CCalcEngine& engine, const TypedNumber& number);
    // The value of a number as the view writes it ("0,3333333333333333"): the result of an entry is the number shown,
    // not the exact one (Windows 7, windows7-history.md part C line 2)
    [[nodiscard]] std::optional<CalcEngine::Rational> valueOf(const std::wstring& written);
    // The value an engine holds now, read through its memory register (which is left as it was)
    [[nodiscard]] static CalcEngine::Rational valueIn(CCalcEngine& engine);
    // The number shown without the separators of Digit grouping: the expression line never groups (Windows 7)
    [[nodiscard]] std::wstring ungrouped(const std::wstring& text) const;
    void binaryOperator(std::uintptr_t opCode);
    void percent();
    // The engine's current value, read exactly through its memory register; the memory is left as it was
    [[nodiscard]] CalcEngine::Rational currentValue();
    // Makes the value the engine's current value, as a memory recall does; the memory is left as it was
    void enterValue(const CalcEngine::Rational& value);
    // The engine's memory, put back after reading (reading takes it out of the engine)
    [[nodiscard]] CalcEngine::Rational memoryValue();

    EngineTexts m_texts;
    Separators m_separators;
    std::unique_ptr<Display> m_display;
    // The display of the evaluation engines, which calculate an entry of the history again without showing it
    std::unique_ptr<Display> m_evaluationDisplay;
    std::shared_ptr<CalculationManager::CalculatorHistory> m_standardHistory;
    std::shared_ptr<CalculationManager::CalculatorHistory> m_scientificHistory;
    std::shared_ptr<CalculationManager::CalculatorHistory> m_programmerHistory;
    std::shared_ptr<CalculationManager::CalculatorHistory> m_statisticsHistory;
    // One engine per view, as Windows 10 has: Standard calculates from left to right with 16 digits, Scientific with
    // precedence and 32 digits, Programmer with precedence on 64-bit integers, Statistics (no operators) with 16 digits.
    // m_engine is the engine of the current view.
    std::unique_ptr<CCalcEngine> m_standardEngine;
    std::unique_ptr<CCalcEngine> m_scientificEngine;
    std::unique_ptr<CCalcEngine> m_programmerEngine;
    std::unique_ptr<CCalcEngine> m_statisticsEngine;
    // The evaluation engines (left to right with 16 digits, precedence with 32), with their own history and display
    std::shared_ptr<CalculationManager::CalculatorHistory> m_evaluationHistory;
    std::unique_ptr<CCalcEngine> m_standardEvaluationEngine;
    std::unique_ptr<CCalcEngine> m_scientificEvaluationEngine;
    CCalcEngine* m_engine = nullptr;
    View m_view = View::Standard;
    bool m_digitGrouping = false;
    bool m_historyShown = false;
    // The history of the Standard and the Scientific view, their selected entries, and the entry or the Statistics
    // value being edited (in the current view)
    std::vector<HistoryEntry> m_standardEntries;
    std::vector<HistoryEntry> m_scientificEntries;
    std::optional<std::size_t> m_standardSelected;
    std::optional<std::size_t> m_scientificSelected;
    std::optional<std::size_t> m_edited;
    std::optional<std::wstring> m_copied;
    bool m_inverse = false;
    AngleUnit m_angleUnit = AngleUnit::Degrees;
    bool m_scientificNotation = false;
    NumberBase m_numberBase = NumberBase::Decimal;
    WordSize m_wordSize = WordSize::Qword;
    // The Statistics view: the list (exact values), its selected item, and the wrapper's own error text ("Enter data
    // to calculate", which the engine does not have); empty while no such error shows
    std::vector<CalcEngine::Rational> m_values;
    std::optional<std::size_t> m_selected;
    std::wstring m_statisticsError;

    // The expression line (Windows 7): the finished part ("2 + 3 *"), whether the last command was a binary operator,
    // the operator pending in the engine, and its left value while it is multiply or divide (for percent)
    std::wstring m_chain;
    bool m_lastWasOperator = false;
    std::uintptr_t m_pendingOperator = 0;
    std::optional<CalcEngine::Rational> m_leftValue;
    // What the expression line shows for the current value after a Windows 7 percent ("100 * 5")
    std::wstring m_percentText;
    // The entry a Standard chain has so far ("2 + 3" while 2+3*4 is typed), which its = completes
    std::wstring m_standardChainEntry;
    // After = the engine's own last part ("4=") is not shown: Windows 7 empties the line
    bool m_afterEquals = false;
    // The result of the last =, while no operator came after it: a percent then takes it as its base
    std::optional<CalcEngine::Rational> m_result;
};

}  // namespace chrono_calc
