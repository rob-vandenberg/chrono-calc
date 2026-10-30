#pragma once

#include <compare>
#include <cstdint>

// --- Version ------------------------------------------------------------
// src/core/calendar.h 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Gregorian calendar with exact whole numbers: day numbers, month lengths, weekdays (D-026)

namespace chrono_calc {

// A day of the Gregorian calendar, also before 1582 (the calendar carried back, as Windows does)
struct Date
{
    int year = 1;
    int month = 1;
    int day = 1;

    friend auto operator<=>(const Date&, const Date&) = default;
};

// Every leap rule of the Gregorian calendar: each fourth year, but not the hundredth unless it is the four-hundredth
bool isLeapYear(int year);
// The number of days of the date's month (its day does not count): 28 to 31; 0 for a month outside 1 to 12
int daysInMonth(const Date& date);
// Whether the year, month and day name a day that exists
bool isValid(const Date& date);
// The number of the day, counted from 1 January 1970 (day 0); only whole numbers, so every result is exact
std::int64_t dayNumber(const Date& date);
// The day with that number
Date dateOfDayNumber(std::int64_t number);
// 0 for Sunday to 6 for Saturday
int weekday(const Date& date);

}  // namespace chrono_calc
