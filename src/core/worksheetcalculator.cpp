#include "core/worksheetcalculator.h"

#include "core/commands.h"
#include "core/unitconversion.h"
#include "core/worksheets.h"

#include "Header Files/Rational.h"
#include "Ratpack/ratpak.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/worksheetcalculator.cpp 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Worksheets' state: the value each one calculates, the text of every value and the message of
//          the last Calculate (D-027)

namespace chrono_calc {

using CalcEngine::Rational;

namespace {

// The precisions a result is worked at: it is taken when two of them give the same 16 digits, so that a result that
// loses digits (a tiny rate, where 1 - (1 + i)^-n cancels) is never shown wrong (D-027)
constexpr std::array<int32_t, 3> precisions{32, 64, 128};

std::size_t indexOf(Worksheet worksheet)
{
    return static_cast<std::size_t>(worksheet);
}

}  // namespace

std::wstring_view messageText(WorksheetMessage message)
{
    switch (message) {
    case WorksheetMessage::Blank:
        return blankText;
    case WorksheetMessage::NotValid:
        return notValidText;
    case WorksheetMessage::UnableToCalculate:
        return unableText;
    case WorksheetMessage::None:
        break;
    }
    return {};
}

WorksheetCalculator::WorksheetCalculator(wchar_t decimal)
    : m_decimal(decimal)
{
    // Every box starts empty (part B line 3)
    for (std::size_t index = 0; index < worksheetCount; ++index) {
        const WorksheetSpec& spec = worksheets().at(index);
        m_states.at(index) = WorksheetState{.chosen = spec.firstChoice,
                                            .texts = std::vector<std::wstring>(spec.values.size()),
                                            .message = WorksheetMessage::None,
                                            .messageValue = 0};
    }
}

void WorksheetCalculator::run(Worksheet worksheet, const CommandTarget& target, std::wstring_view text)
{
    WorksheetState& state = m_states.at(indexOf(worksheet));
    const WorksheetSpec& spec = worksheetSpec(worksheet);
    switch (target.kind) {
    case CommandKind::WorksheetChoose:
        if (target.index < spec.choices.size()) {
            state.chosen = spec.choices.at(target.index);
            state.message = WorksheetMessage::None;
        }
        break;
    case CommandKind::WorksheetValue:
        // Only a value that has a box; the result box is read-only
        if (target.index < state.texts.size() && target.index != state.chosen) {
            state.texts.at(target.index) = acceptedNumberText(text, m_decimal);
            state.message = WorksheetMessage::None;
        }
        break;
    case CommandKind::WorksheetCalculate:
        calculate(worksheet);
        break;
    default:
        break;
    }
}

// The boxes are read in their order: the first one that holds no valid number stops Calculate and keeps the result;
// otherwise the first empty one stops it and empties the result (windows7-worksheets.md part B line 4, part D)
void WorksheetCalculator::calculate(Worksheet worksheet)
{
    WorksheetState& state = m_states.at(indexOf(worksheet));
    std::vector<Rational> values(state.texts.size(), Rational(0));
    std::optional<std::size_t> firstEmpty;
    bool exponentWritten = false;
    state.message = WorksheetMessage::None;
    for (const std::size_t value : boxValues(worksheet)) {
        const std::wstring& text = state.texts.at(value);
        if (text.empty()) {
            firstEmpty = firstEmpty ? firstEmpty : value;
            continue;
        }
        const std::optional<Rational> number = boxNumber(text);
        if (!number) {
            state.message = WorksheetMessage::NotValid;
            state.messageValue = value;
            return;
        }
        values.at(value) = *number;
        exponentWritten = exponentWritten || text.find(L'e') != std::wstring::npos;
    }
    if (firstEmpty) {
        state.message = WorksheetMessage::Blank;
        state.messageValue = *firstEmpty;
        state.texts.at(state.chosen).clear();
        return;
    }
    // The number library's radix and precision are global: decimal here, at each precision in turn; the caller gives
    // the view's own back after
    std::optional<std::wstring> previous;
    std::optional<std::wstring> agreed;
    bool agreement = false;
    for (std::size_t round = 0; round < precisions.size() && !agreement; ++round) {
        ChangeConstants(10, precisions.at(round));
        const std::optional<Rational> result = calculateWorksheet(worksheet, state.chosen, values);
        // A number typed with an exponent makes the result written with one too: 1e5 / 1 is "1,e+5" (part E line 1).
        // The number library reports what it cannot write by throwing its error code.
        std::optional<std::wstring> text;
        try {
            text = result ? std::optional<std::wstring>(writtenNumber(*result, exponentWritten, m_decimal)) : std::nullopt;
        } catch (const uint32_t /*error*/) {
            text.reset();
        }
        if (round > 0 && text == previous) {
            agreement = true;
            agreed = text;
        }
        previous = std::move(text);
    }
    if (!agreed) {
        state.message = WorksheetMessage::UnableToCalculate;
        state.messageValue = state.chosen;
        state.texts.at(state.chosen).clear();
        return;
    }
    state.texts.at(state.chosen) = *agreed;
}

// Windows 7's reading of a box (part E line 1), as in Unit conversion, except that no value may be negative ("-0" and
// "-" too)
std::optional<Rational> WorksheetCalculator::boxNumber(std::wstring_view text) const
{
    if (text.starts_with(L'-')) {
        return std::nullopt;
    }
    return typedNumber(text, m_decimal);
}

const WorksheetState& WorksheetCalculator::state(Worksheet worksheet) const
{
    return m_states.at(indexOf(worksheet));
}

std::vector<std::size_t> WorksheetCalculator::boxValues(Worksheet worksheet) const
{
    const WorksheetState& state = m_states.at(indexOf(worksheet));
    std::vector<std::size_t> values;
    for (std::size_t value = 0; value < state.texts.size(); ++value) {
        if (value != state.chosen) {
            values.push_back(value);
        }
    }
    return values;
}

std::size_t WorksheetCalculator::chosenItem(Worksheet worksheet) const
{
    const std::vector<std::size_t>& choices = worksheetSpec(worksheet).choices;
    for (std::size_t item = 0; item < choices.size(); ++item) {
        if (choices.at(item) == m_states.at(indexOf(worksheet)).chosen) {
            return item;
        }
    }
    return 0;
}

}  // namespace chrono_calc
