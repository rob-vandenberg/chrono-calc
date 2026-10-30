#pragma once

#include "core/commands.h"
#include "core/worksheets.h"

#include "Header Files/Rational.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/worksheetcalculator.h 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Worksheets' state: the value each one calculates, the text of every value and the message of
//          the last Calculate (D-027)

namespace chrono_calc {

// What the last Calculate said (Windows 7 shows it in a balloon, windows7-worksheets.md part B line 4): nothing, a box
// left empty, a box that holds no valid number, or no answer
enum class WorksheetMessage : std::uint8_t
{
    None,
    Blank,
    NotValid,
    UnableToCalculate,
};

// Windows 7's texts of the messages
inline constexpr std::wstring_view blankText = L"This value cannot be blank.";
inline constexpr std::wstring_view notValidText = L"This value is not valid.";
inline constexpr std::wstring_view unableText = L"Unable to calculate";

std::wstring_view messageText(WorksheetMessage message);

// One worksheet as the panel shows it: the value it calculates (an index into its values), the text of every value (the
// chosen one's text is the result box, the others are the boxes), and the message of the last Calculate with the value
// whose box it points at (the result box for UnableToCalculate)
struct WorksheetState
{
    std::size_t chosen = 0;
    std::vector<std::wstring> texts;
    WorksheetMessage message = WorksheetMessage::None;
    std::size_t messageValue = 0;
};

// The four Worksheets (tests/reference/windows7-worksheets.md). Each keeps its chosen value, its texts and its result
// while another worksheet, panel or view shows (part B line 7). Choosing another value keeps every text: the old
// result becomes a box and the newly chosen value's text moves into the result box (part B line 1). A result appears
// only with Calculate; changing a box keeps the result (part B line 2).
class WorksheetCalculator
{
public:
    // The regional decimal symbol, which the boxes take and the result is written with
    explicit WorksheetCalculator(wchar_t decimal);

    // The panel's commands (CommandKind::Worksheet...) on the worksheet shown; a box's text in "text"
    void run(Worksheet worksheet, const CommandTarget& target, std::wstring_view text);

    [[nodiscard]] const WorksheetState& state(Worksheet worksheet) const;
    // The values that have a box, in the order of the boxes: the worksheet's values without the chosen one
    [[nodiscard]] std::vector<std::size_t> boxValues(Worksheet worksheet) const;
    // The index in the list of the value calculated
    [[nodiscard]] std::size_t chosenItem(Worksheet worksheet) const;

private:
    void calculate(Worksheet worksheet);
    // The number in a box: nothing when it is not valid (no number, too many digits, a minus)
    [[nodiscard]] std::optional<CalcEngine::Rational> boxNumber(std::wstring_view text) const;

    wchar_t m_decimal;
    std::array<WorksheetState, worksheetCount> m_states;
};

}  // namespace chrono_calc
