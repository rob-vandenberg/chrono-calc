#pragma once

#include "core/calendar.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/datecalculation.h 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Date calculation panel's two calculations, on whole day numbers (D-026)

namespace chrono_calc {

// The first and last day the panel's date pickers take (tests/reference/windows7-datecalculation.md part C line 4)
inline constexpr Date firstDate{.year = 1601, .month = 1, .day = 1};
inline constexpr Date lastDate{.year = 9999, .month = 12, .day = 31};

// The difference between two days as Windows 7 counts it (part D): the whole months from the earlier day, each month
// counted from that day (31 January plus one month is 28 February, its last day), as years and months; the days left
// as weeks and days; and the whole difference in days. The order of the two days does not matter.
struct DateDifference
{
    std::int64_t years = 0;
    std::int64_t months = 0;
    std::int64_t weeks = 0;
    std::int64_t days = 0;
    std::int64_t totalDays = 0;
};

DateDifference differenceBetween(const Date& first, const Date& second);
// "1 year, 2 months, 5 days": the parts that are not 0, singular or plural; "Same dates" for the same day
std::wstring differenceText(const DateDifference& difference);
// "429 days", "1 day"; "Same dates" for the same day
std::wstring totalDaysText(const DateDifference& difference);

// The day the months later (or earlier, for a negative number); a day beyond the end of that month becomes its last
// day (31 January plus one month is 28 February)
Date addMonths(const Date& date, std::int64_t months);

// Add or subtract (part E): adding moves the years, then the months, then the days; subtracting the reverse: the days,
// the months, the years; after the years and after the months a day beyond the month's end becomes its last day. So
// 29 February 2024 plus one year and one month is 28 March 2025, and 31 March minus one month and one day is
// 28 February, as in Windows 7. Nothing when the day lies outside the first and last date ("Date out of bound").
std::optional<Date> shiftDate(const Date& date, bool subtract, std::int64_t years, std::int64_t months, std::int64_t days);
inline constexpr std::wstring_view outOfBoundText = L"Date out of bound";

// A day written "2026-10-30", when it exists and lies within the first and last date; nothing otherwise
std::optional<Date> parseDate(std::wstring_view text);
// "2026-10-30"
std::wstring isoText(const Date& date);

// Today on this computer's clock and time zone
Date today();

}  // namespace chrono_calc
