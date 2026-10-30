#include "core/datecalculator.h"

#include "core/calendar.h"
#include "core/commands.h"
#include "core/datecalculation.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/datecalculator.cpp 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Date calculation panel's state: the mode, the dates, Add or Subtract, the three numbers and the
//          results (D-026)

namespace chrono_calc {

namespace {

constexpr std::int64_t fieldValues = 1000;

std::size_t indexOf(DateField field)
{
    return static_cast<std::size_t>(field);
}

}  // namespace

DateCalculator::DateCalculator(const Date& today)
    : m_differenceFrom(today)
    , m_differenceTo(today)
    , m_shiftFrom(today)
{
}

void DateCalculator::run(const CommandTarget& target, std::wstring_view text)
{
    switch (target.kind) {
    case CommandKind::DateMode:
        if (target.index <= static_cast<std::size_t>(DateMode::AddOrSubtract)) {
            m_mode = static_cast<DateMode>(target.index);
        }
        break;
    case CommandKind::DateDifferenceFrom:
    case CommandKind::DateDifferenceTo:
        if (setDate(target.kind == CommandKind::DateDifferenceFrom ? m_differenceFrom : m_differenceTo, text)) {
            m_differenceText.clear();
            m_totalDaysText.clear();
        }
        break;
    case CommandKind::DateShiftFrom:
        if (setDate(m_shiftFrom, text)) {
            m_shifted = {};
        }
        break;
    case CommandKind::DateAdd:
    case CommandKind::DateSubtract:
        if (m_subtract != (target.kind == CommandKind::DateSubtract)) {
            m_subtract = target.kind == CommandKind::DateSubtract;
            m_shifted = {};
        }
        break;
    case CommandKind::DateField:
        setField(static_cast<DateField>(target.index), text);
        break;
    case CommandKind::DateFieldUp:
    case CommandKind::DateFieldDown:
        stepField(static_cast<DateField>(target.index), target.kind == CommandKind::DateFieldUp);
        break;
    case CommandKind::DateCalculate:
        calculate();
        break;
    default:
        break;
    }
}

// A date the panel does not take (no such day, before 1601 or after 9999) changes nothing (part C lines 3 and 4); the
// same date again keeps the result. Whether the date changed.
bool DateCalculator::setDate(Date& date, std::wstring_view text)
{
    const std::optional<Date> parsed = parseDate(text);
    if (!parsed || *parsed == date) {
        return false;
    }
    date = *parsed;
    return true;
}

// The box takes digits only, at most three ("5a" is 5, "99999" is 999); it may be empty (part E line 1)
void DateCalculator::setField(DateField field, std::wstring_view text)
{
    std::wstring taken;
    for (const wchar_t character : text) {
        if (character >= L'0' && character <= L'9' && taken.size() < mostDigits) {
            taken += character;
        }
    }
    std::wstring& shown = m_fields.at(indexOf(field));
    if (taken != shown) {
        shown = taken;
        m_shifted = {};
    }
}

void DateCalculator::stepField(DateField field, bool up)
{
    const std::int64_t value = (valueOf(field) + (up ? 1 : fieldValues - 1)) % fieldValues;
    m_fields.at(indexOf(field)) = std::to_wstring(value);
    m_shifted = {};
}

std::int64_t DateCalculator::valueOf(DateField field) const
{
    // An empty box counts as 0 (part E line 1)
    std::int64_t value = 0;
    for (const wchar_t digit : m_fields.at(indexOf(field))) {
        value = (value * 10) + (digit - L'0');
    }
    return value;
}

void DateCalculator::calculate()
{
    if (m_mode == DateMode::Difference) {
        const DateDifference difference = differenceBetween(m_differenceFrom, m_differenceTo);
        m_differenceText = chrono_calc::differenceText(difference);
        m_totalDaysText = chrono_calc::totalDaysText(difference);
        return;
    }
    m_shifted = {.shown = true,
                 .date = shiftDate(m_shiftFrom, m_subtract, valueOf(DateField::Years), valueOf(DateField::Months),
                                   valueOf(DateField::Days))};
}

DateMode DateCalculator::mode() const
{
    return m_mode;
}

const Date& DateCalculator::differenceFrom() const
{
    return m_differenceFrom;
}

const Date& DateCalculator::differenceTo() const
{
    return m_differenceTo;
}

const Date& DateCalculator::shiftFrom() const
{
    return m_shiftFrom;
}

bool DateCalculator::isSubtract() const
{
    return m_subtract;
}

const std::wstring& DateCalculator::field(DateField field) const
{
    return m_fields.at(indexOf(field));
}

const std::wstring& DateCalculator::differenceText() const
{
    return m_differenceText;
}

const std::wstring& DateCalculator::totalDaysText() const
{
    return m_totalDaysText;
}

const ShiftedDate& DateCalculator::shifted() const
{
    return m_shifted;
}

}  // namespace chrono_calc
