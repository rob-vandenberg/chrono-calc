#pragma once

#include "core/calendar.h"
#include "core/commands.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/datecalculator.h 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Date calculation panel's state: the mode, the dates, Add or Subtract, the three numbers and the
//          results (D-026)

namespace chrono_calc {

// The two calculations of the panel, in the order of its list (windows7-datecalculation.md part B)
enum class DateMode : std::uint8_t
{
    Difference,
    AddOrSubtract,
};

// The three numbers of Add or subtract: Year(s), Month(s), Day(s)
enum class DateField : std::uint8_t
{
    Years,
    Months,
    Days,
};

// What the Add or subtract result box shows: nothing, a day (the window writes it in the regional long date), or
// "Date out of bound"
struct ShiftedDate
{
    bool shown = false;
    std::optional<Date> date;
};

// The Date calculation panel (tests/reference/windows7-datecalculation.md). Each mode keeps its own dates, numbers and
// results, also while the other mode or another panel shows (part B line 2, part G). A result appears only with
// Calculate; changing a date, a number or Add/Subtract clears that mode's result (part D line 1, part E line 3).
class DateCalculator
{
public:
    // Every date starts at today (part C line 1)
    explicit DateCalculator(const Date& today);

    // The panel's commands (CommandKind::Date...); a date or number given as text in "text"
    void run(const CommandTarget& target, std::wstring_view text);

    [[nodiscard]] DateMode mode() const;
    [[nodiscard]] const Date& differenceFrom() const;
    [[nodiscard]] const Date& differenceTo() const;
    [[nodiscard]] const Date& shiftFrom() const;
    [[nodiscard]] bool isSubtract() const;
    // The text of a number box: up to three digits, possibly empty (part E line 1)
    [[nodiscard]] const std::wstring& field(DateField field) const;
    // The Difference results: "1 year, 2 months, 5 days" and "429 days"; empty before Calculate
    [[nodiscard]] const std::wstring& differenceText() const;
    [[nodiscard]] const std::wstring& totalDaysText() const;
    [[nodiscard]] const ShiftedDate& shifted() const;

    // The most digits a number box takes
    static constexpr std::size_t mostDigits = 3;

private:
    [[nodiscard]] static bool setDate(Date& date, std::wstring_view text);
    void setField(DateField field, std::wstring_view text);
    // The arrows beside a number box (and Up, Down in it): one more or less, from 999 to 0 and back (part E line 1)
    void stepField(DateField field, bool up);
    void calculate();
    [[nodiscard]] std::int64_t valueOf(DateField field) const;

    DateMode m_mode = DateMode::Difference;
    Date m_differenceFrom;
    Date m_differenceTo;
    Date m_shiftFrom;
    bool m_subtract = false;
    std::array<std::wstring, 3> m_fields{L"0", L"0", L"0"};
    std::wstring m_differenceText;
    std::wstring m_totalDaysText;
    ShiftedDate m_shifted;
};

}  // namespace chrono_calc
