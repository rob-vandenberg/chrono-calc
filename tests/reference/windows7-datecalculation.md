# Windows 7 Calculator: Date calculation reference

Measured on 2026-10-30 on the owner's Windows computer, in the original Windows 7 Calculator (calc1.exe), through real
mouse clicks and keys (task M7); every date typed into its date box and read back. Physical pixels at 100%. Regional
settings there: en-NL, short date dd/MM/yyyy, long date "dddd, d MMMM yyyy", first day of the week Monday; today was
Friday 30 October 2026. The controls are drawn by that computer's Windows 10 visual style. Lines marked "Chrono Calc"
say where we differ, and why (D-026).

## A. Layout and look

1. View > Date calculation (Ctrl+E) shows the panel in the same frame as Unit conversion (windows7-unitconversion.md
   part A line 1): client sizes Standard 731 x 331, Scientific 975 x 331, Programmer 975 x 413, Statistics 731 x 456,
   Standard with History 731 x 456; the panel Standard 264, 13, 454, 303; Scientific 508, 13, 454, 303; Programmer 508,
   13, 454, 386; Statistics 264, 13, 454, 427; Standard with History 264, 13, 454, 428.
2. Inside (Standard, client coordinates). Always: the label "Select the date calculation you want" 276, 26, 420, 24; the
   mode list (a drop-down list, not sorted) 276, 54, 432, 25. Below it the parts of one mode (an inner dialog at 268, 94,
   444, 215).
   1. Difference: "From" 276, 102, 69, 17; its date box 351, 94, 120, 30; "To" 525, 102, 63, 17; its date box 587, 94,
      120, 30; "Difference (years, months, weeks, days)" 276, 128, 420, 17; its box (read-only) 276, 155, 432, 28;
      "Difference (days)" 276, 196, 420, 17; its box (read-only) 276, 218, 432, 28; "Calculate" (the default button)
      609, 277, 99, 28.
   2. Add or subtract: "From" 276, 102, 63, 17; its date box 344, 94, 120, 30; the radio buttons "Add" 480, 98, 97, 21
      and "Subtract" 577, 98, 125, 21; "Year(s)" 276, 139, 68, 21 with its box 344, 133, 42, 28 and arrows 384, 133, 18,
      28; "Month(s)" 408, 139, 94, 21 with 506, 133, 42, 28 and 546, 133, 18, 28; "Day(s)" 571, 139, 72, 21 with 649,
      133, 42, 28 and 689, 133, 18, 28; "Date" 276, 179, 403, 17; its box (read-only) 276, 203, 432, 28; "Calculate"
      609, 277, 99, 28.
   3. No label has an access key (no underlined letter).
   4. Chrono Calc: the number boxes are 40 wide, so that they end where their arrows begin (Windows 7 lets the arrows
      overlap the box by 2 px).
3. Colours and fonts: Segoe UI 13 px everywhere. The panel's frame #889AAD, inside white, no line inside. Labels #1E395B
   ("From" 9 px ink). Date boxes: edge #7A7A7A, white, text #000000 (12 px ink), the drop-down arrow #4D6185 at the
   right. Result boxes: edge #7A7A7A, #F0F0F0, text #000000. Calculate: face #F0F0F0 / #E1E1E1, the default button's
   edge #0078D7, text #000000. The mode list as the Unit conversion lists.
4. Ctrl+E while the panel shows keeps it (no toggle); Ctrl+E from Unit conversion changes to Date calculation; Ctrl+F4
   (Basic) hides it. After Ctrl+E the keyboard focus is on the mode list. Basic, Unit conversion and Date calculation
   share one dot and are never greyed.

## B. The mode list

1. "Calculate the difference between two dates", "Add or subtract days to a specified date", in this order; the first
   the first time.
2. A mode change keeps everything of both modes: the dates, results, Add or Subtract, the numbers. Each mode has its own
   From date.

## C. The date boxes

1. They show the short date ("30/10/2026"), start at today (all three), and have no check box.
   Chrono Calc: the regional short date, always with a four-digit year.
2. F4 opens the calendar below the box, aligned to its right edge; Escape closes it with the date unchanged. Its look,
   today's mark and its keys were not measured (Chrono Calc: Qt's calendar in our colours).
3. Left and Right move between day, month and year (Left stops at the day). Up and Down change the part and go round
   within it: day 01 Down is 31 in January, day 31 Up is 01. Digits are typed per part; one digit waits for the second
   or a move. A day beyond the month's end is refused (31 typed in April keeps the old day). 29/02/2024 with Up on the
   year is 28/02/2025.
4. The first date is 01/01/1601, the last 31/12/9999 (1601, 1752, 1753 and 1900 are taken; 1600 typed is refused). Down
   on the year at 1601 stays 1601. Up on the day at 31/12/9999 goes round to 01/12/9999.

## D. The difference between two dates

1. The results come only with Calculate, or Enter in a date box (which also moves the keyboard focus to the first result
   box). Changing either date empties both results.
   Chrono Calc: the calendar opened and closed without a change keeps the results (Windows 7 empties them, follow-up
   line 11); nothing changed, so the results still hold.
2. Results (line 1 | line 2):

| # | From | To | Difference (years, months, weeks, days) | Difference (days) |
|---|---|---|---|---|
| 1 | 01/01/2026 | 01/01/2026 | Same dates | Same dates |
| 2 | 01/01/2026 | 02/01/2026 | 1 day | 1 day |
| 3 | 01/01/2026 | 08/01/2026 | 1 week | 7 days |
| 4 | 01/01/2026 | 15/01/2026 | 2 weeks | 14 days |
| 5 | 31/01/2026 | 28/02/2026 | 1 month | 28 days |
| 6 | 31/01/2026 | 01/03/2026 | 1 month, 1 day | 29 days |
| 7 | 30/01/2026 | 28/02/2026 | 1 month | 29 days |
| 8 | 28/01/2026 | 28/02/2026 | 1 month | 31 days |
| 9 | 29/02/2024 | 28/02/2025 | 1 year | 365 days |
| 10 | 29/02/2024 | 01/03/2025 | 1 year, 1 day | 366 days |
| 11 | 29/02/2024 | 29/02/2028 | 4 years | 1461 days |
| 12 | 31/03/2023 | 30/04/2023 | 1 month | 30 days |
| 13 | 31/12/2023 | 31/12/2024 | 1 year | 366 days |
| 14 | 31/05/2026 | 30/06/2026 | 1 month | 30 days |
| 15 | 15/03/2026 | 14/05/2026 | 1 month, 4 weeks, 1 day | 60 days |
| 16 | 15/01/2026 | 20/03/2027 | 1 year, 2 months, 5 days | 429 days |
| 17 | 25/12/2025 | 01/01/2026 | 1 week | 7 days |
| 18 | 28/02/1900 | 01/03/1900 | 1 day | 1 day |
| 19 | 01/01/2000 | 30/10/2026 | 26 years, 9 months, 4 weeks, 1 day | 9799 days |
| 20 | 30/10/2026 | 01/01/2000 | 26 years, 9 months, 4 weeks, 1 day | 9799 days |
| 21 | 28/03/2026 | 30/03/2026 | 2 days | 2 days |
| 22 | 01/01/1601 | 31/12/9999 | 8398 years, 11 months, 4 weeks, 2 days | 3067670 days |
| 23 | 28/02/2026 | 31/03/2026 | 1 month, 3 days | 31 days |
| 24 | 31/01/2025 | 31/03/2025 | 2 months | 59 days |

3. The parts that are 0 are left out, the rest joined by ", ", singular and plural; the second line is the whole
   difference in days, never grouped; the same day gives "Same dates" on both lines; To before From gives the same
   result (no sign). The summer time change has no effect (21). The months are counted from the earlier day, each
   month's day brought back to the month's end (5, 7, 24), the days left as weeks and days.

## E. Add or subtract

1. The number boxes take digits only, at most three (a letter or "-" is refused, "5a" gives 5, "99999" gives 999), start
   at 0 and may be empty (Calculate takes an empty box as 0; the box stays empty). Their arrows, and Up and Down in the
   box, step by one from 0 to 999 and round: Up at 999 is 0, Down at 0 is 999; an empty box Up is 1, Down is 999.
2. Results (From, Add or Subtract, years, months, days):

| # | From | | Y | M | D | Date |
|---|---|---|---|---|---|---|
| 1 | 31/01/2026 | add | 0 | 1 | 0 | Saturday, 28 February 2026 |
| 2 | 31/01/2028 | add | 0 | 1 | 0 | Tuesday, 29 February 2028 |
| 3 | 30/01/2026 | add | 0 | 1 | 0 | Saturday, 28 February 2026 |
| 4 | 29/02/2024 | add | 1 | 0 | 0 | Friday, 28 February 2025 |
| 5 | 29/02/2024 | subtract | 1 | 0 | 0 | Tuesday, 28 February 2023 |
| 6 | 29/02/2024 | add | 4 | 0 | 0 | Tuesday, 29 February 2028 |
| 7 | 31/03/2026 | subtract | 0 | 1 | 0 | Saturday, 28 February 2026 |
| 8 | 31/01/2026 | add | 0 | 1 | 1 | Sunday, 1 March 2026 |
| 9 | 31/03/2026 | subtract | 0 | 1 | 1 | Saturday, 28 February 2026 |
| 10 | 30/10/2026 | add | 0 | 0 | 0 | Friday, 30 October 2026 |
| 11 | 31/12/2026 | add | 0 | 0 | 1 | Friday, 1 January 2027 |
| 12 | 01/03/2026 | subtract | 0 | 0 | 1 | Saturday, 28 February 2026 |
| 13 | 30/10/2026 | add | 0 | 13 | 0 | Tuesday, 30 November 2027 |
| 14 | 30/10/2026 | subtract | 0 | 0 | 365 | Thursday, 30 October 2025 |
| 15 | 31/08/2026 | add | 0 | 6 | 0 | Sunday, 28 February 2027 |
| 16 | 30/10/2026 | add | 999 | 999 | 999 | Thursday, 26 October 3111 |
| 17 | 31/12/9999 | add | 0 | 0 | 1 | Date out of bound |
| 18 | 31/12/9999 | add | 1 | 0 | 0 | Date out of bound |
| 19 | 01/01/1601 | subtract | 0 | 0 | 1 | Date out of bound |
| 20 | 30/10/2026 | add | 1 | 2 | 3 | Sunday, 2 January 2028 |
| 21 | 30/10/2026 | subtract | 1 | 2 | 3 | Wednesday, 27 August 2025 |
| 22 | 29/02/2024 | add | 1 | 1 | 0 | Friday, 28 March 2025 |
| 23 | 29/02/2024 | subtract | 1 | 1 | 0 | Sunday, 29 January 2023 |
| 24 | 01/03/2026 | subtract | 0 | 1 | 1 | Wednesday, 28 January 2026 |
| 25 | 28/02/2026 | add | 0 | 1 | 1 | Sunday, 29 March 2026 |
| 26 | 31/05/2026 | subtract | 0 | 1 | 30 | Wednesday, 1 April 2026 |
| 27 | 31/01/2026 | add | 1 | 1 | 0 | Sunday, 28 February 2027 |
| 28 | 30/12/9999 | add | 0 | 0 | 1 | Friday, 31 December 9999 |
| 29 | 01/12/9999 | add | 0 | 1 | 0 | Date out of bound |
| 30 | 31/03/2026 | subtract | 0 | 0 | 0 | Tuesday, 31 March 2026 |

3. The order: adding moves the years, then the months, then the days; subtracting the reverse (days, months, years);
   after each step a day beyond the month's end becomes its last day (22: 29/02/2024 +1 year is 28/02/2025, +1 month
   28/03/2025; 9: 31/03 -1 day is 30/03, -1 month 28/02). The result is written in the regional long date.
   Chrono Calc: the regional long date of the computer.
4. The result comes only with Calculate, or Enter in a number box or the date box (the keyboard focus then goes to the
   result box). Changing a number, the date or Add/Subtract empties the result. The arrows on the radio buttons choose
   (Left and Up: Add; Right and Down: Subtract) and empty the result.

## F. Keys and focus

1. Tab order. Difference: From, To, the first result, the second result, Calculate, the calculator, the mode list, From
   again. Add or subtract: the date, the chosen radio button (Add or Subtract), Year(s), Month(s), Day(s), the result,
   Calculate, the calculator, the mode list.
2. Enter in a date or number box is Calculate. Escape in a number box does nothing (the focus stays). "+" typed into a
   number box and "*" into a date box are refused and never reach the calculator.

## G. What is kept

1. Everything of the panel (mode, dates, Add or Subtract, numbers, results) is kept over every view change, Basic and
   back, Unit conversion and back, and a mode change; Unit conversion keeps its own too.
2. The registry (HKCU\Software\Microsoft\Calc) holds DateTime=0/1 for the panel shown, nothing of its dates, mode or
   numbers. A restart was not measured; Chrono Calc remembers the panel shown, as Windows 7's DateTime says, not its
   choices (as with Unit conversion, D-025).
