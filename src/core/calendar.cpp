#include "core/calendar.h"

#include <cstdint>

// --- Version ------------------------------------------------------------
// src/core/calendar.cpp 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Gregorian calendar with exact whole numbers: day numbers, month lengths, weekdays (D-026)

namespace chrono_calc {

namespace {

// The calendar in eras of 400 years, each 146097 days long, with the year starting on 1 March, so that the leap day is
// the last day of a year (the method of Howard Hinnant's "chrono-Compatible Low-Level Date Algorithms")
constexpr std::int64_t daysPerEra = 146097;
constexpr std::int64_t yearsPerEra = 400;
// The day number of 1 March of the year 0, counted from 1 January 1970
constexpr std::int64_t marchOfYearZero = 719468;

}  // namespace

bool isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

int daysInMonth(const Date& date)
{
    switch (date.month) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
        return 31;
    case 4:
    case 6:
    case 9:
    case 11:
        return 30;
    case 2:
        return isLeapYear(date.year) ? 29 : 28;
    default:
        return 0;
    }
}

bool isValid(const Date& date)
{
    return date.month >= 1 && date.month <= 12 && date.day >= 1 && date.day <= daysInMonth(date);
}

std::int64_t dayNumber(const Date& date)
{
    const std::int64_t year = date.month <= 2 ? std::int64_t{date.year} - 1 : std::int64_t{date.year};
    const std::int64_t era = (year >= 0 ? year : year - (yearsPerEra - 1)) / yearsPerEra;
    const std::int64_t yearOfEra = year - (era * yearsPerEra);
    const std::int64_t monthFromMarch = date.month > 2 ? date.month - 3 : date.month + 9;
    const std::int64_t dayOfYear = (((153 * monthFromMarch) + 2) / 5) + date.day - 1;
    const std::int64_t dayOfEra = (yearOfEra * 365) + (yearOfEra / 4) - (yearOfEra / 100) + dayOfYear;
    return (era * daysPerEra) + dayOfEra - marchOfYearZero;
}

Date dateOfDayNumber(std::int64_t number)
{
    const std::int64_t shifted = number + marchOfYearZero;
    const std::int64_t era = (shifted >= 0 ? shifted : shifted - (daysPerEra - 1)) / daysPerEra;
    const std::int64_t dayOfEra = shifted - (era * daysPerEra);
    const std::int64_t yearOfEra = (dayOfEra - (dayOfEra / 1460) + (dayOfEra / 36524) - (dayOfEra / (daysPerEra - 1))) / 365;
    const std::int64_t dayOfYear = dayOfEra - ((365 * yearOfEra) + (yearOfEra / 4) - (yearOfEra / 100));
    const std::int64_t monthFromMarch = ((5 * dayOfYear) + 2) / 153;
    const std::int64_t day = dayOfYear - (((153 * monthFromMarch) + 2) / 5) + 1;
    const std::int64_t month = monthFromMarch < 10 ? monthFromMarch + 3 : monthFromMarch - 9;
    const std::int64_t year = yearOfEra + (era * yearsPerEra) + (month <= 2 ? 1 : 0);
    return {.year = static_cast<int>(year), .month = static_cast<int>(month), .day = static_cast<int>(day)};
}

int weekday(const Date& date)
{
    // 1 January 1970 was a Thursday (4)
    const std::int64_t number = dayNumber(date);
    const std::int64_t shifted = (number + 4) % 7;
    return static_cast<int>(shifted < 0 ? shifted + 7 : shifted);
}

}  // namespace chrono_calc
