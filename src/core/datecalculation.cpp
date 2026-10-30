#include "core/datecalculation.h"

#include "core/calendar.h"

#include <algorithm>
#include <cstdint>
#include <ctime>
#include <optional>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/datecalculation.cpp 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Date calculation panel's two calculations, on whole day numbers (D-026)

namespace chrono_calc {

namespace {

constexpr std::int64_t monthsPerYear = 12;
constexpr std::int64_t daysPerWeek = 7;

// Division that rounds down, also for a negative number
std::int64_t floorDivision(std::int64_t number, std::int64_t divisor)
{
    const std::int64_t quotient = number / divisor;
    return (number % divisor != 0 && (number < 0) != (divisor < 0)) ? quotient - 1 : quotient;
}

// "1 day", "2 days"
std::wstring countText(std::int64_t count, std::wstring_view one, std::wstring_view many)
{
    return std::to_wstring(count) + L' ' + std::wstring(count == 1 ? one : many);
}

constexpr std::wstring_view sameDatesText = L"Same dates";

// The number of a run of digits, nothing when it holds anything else
std::optional<int> numberOf(std::wstring_view digits)
{
    if (digits.empty()) {
        return std::nullopt;
    }
    int number = 0;
    for (const wchar_t digit : digits) {
        if (digit < L'0' || digit > L'9') {
            return std::nullopt;
        }
        number = (number * 10) + static_cast<int>(digit - L'0');
    }
    return number;
}

}  // namespace

DateDifference differenceBetween(const Date& first, const Date& second)
{
    const Date& from = first < second ? first : second;
    const Date& to = first < second ? second : first;
    // The months from the earlier day: as many as end on or before the later day
    std::int64_t months = ((std::int64_t{to.year} - from.year) * monthsPerYear) + (to.month - from.month);
    if (addMonths(from, months) > to) {
        --months;
    }
    const std::int64_t rest = dayNumber(to) - dayNumber(addMonths(from, months));
    return {.years = months / monthsPerYear,
            .months = months % monthsPerYear,
            .weeks = rest / daysPerWeek,
            .days = rest % daysPerWeek,
            .totalDays = dayNumber(to) - dayNumber(from)};
}

std::wstring differenceText(const DateDifference& difference)
{
    if (difference.totalDays == 0) {
        return std::wstring(sameDatesText);
    }
    std::wstring text;
    const auto addPart = [&text](std::int64_t count, std::wstring_view one, std::wstring_view many) {
        if (count == 0) {
            return;
        }
        if (!text.empty()) {
            text += L", ";
        }
        text += countText(count, one, many);
    };
    addPart(difference.years, L"year", L"years");
    addPart(difference.months, L"month", L"months");
    addPart(difference.weeks, L"week", L"weeks");
    addPart(difference.days, L"day", L"days");
    return text;
}

std::wstring totalDaysText(const DateDifference& difference)
{
    if (difference.totalDays == 0) {
        return std::wstring(sameDatesText);
    }
    return countText(difference.totalDays, L"day", L"days");
}

Date addMonths(const Date& date, std::int64_t months)
{
    const std::int64_t index = (std::int64_t{date.year} * monthsPerYear) + (date.month - 1) + months;
    const std::int64_t year = floorDivision(index, monthsPerYear);
    Date shifted{.year = static_cast<int>(year), .month = static_cast<int>(index - (year * monthsPerYear) + 1), .day = 1};
    shifted.day = std::min(date.day, daysInMonth(shifted));
    return shifted;
}

std::optional<Date> shiftDate(const Date& date, bool subtract, std::int64_t years, std::int64_t months, std::int64_t days)
{
    // Each step on its own, the day brought back to the month's end after the years and after the months
    const Date shifted = subtract
                             ? addMonths(addMonths(dateOfDayNumber(dayNumber(date) - days), -months), -(years * monthsPerYear))
                             : dateOfDayNumber(dayNumber(addMonths(addMonths(date, years * monthsPerYear), months)) + days);
    if (shifted < firstDate || shifted > lastDate) {
        return std::nullopt;
    }
    return shifted;
}

std::optional<Date> parseDate(std::wstring_view text)
{
    // "yyyy-mm-dd"
    constexpr std::size_t length = 10;
    if (text.size() != length || text[4] != L'-' || text[7] != L'-') {
        return std::nullopt;
    }
    const std::optional<int> year = numberOf(text.substr(0, 4));
    const std::optional<int> month = numberOf(text.substr(5, 2));
    const std::optional<int> day = numberOf(text.substr(8, 2));
    if (!year || !month || !day) {
        return std::nullopt;
    }
    const Date date{.year = *year, .month = *month, .day = *day};
    if (!isValid(date) || date < firstDate || date > lastDate) {
        return std::nullopt;
    }
    return date;
}

std::wstring isoText(const Date& date)
{
    const auto twoDigits = [](int number) { return (number < 10 ? std::wstring(L"0") : std::wstring()) + std::to_wstring(number); };
    return std::to_wstring(date.year) + L'-' + twoDigits(date.month) + L'-' + twoDigits(date.day);
}

Date today()
{
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    constexpr int yearsBefore = 1900;
    return {.year = local.tm_year + yearsBefore, .month = local.tm_mon + 1, .day = local.tm_mday};
}

}  // namespace chrono_calc
