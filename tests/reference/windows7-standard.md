# Windows 7 Calculator: Standard view reference

Measured on 2026-10-30 on the owner's Windows computer, in the original Windows 7 Calculator (calc1.exe), by reading its
window through UI Automation and pressing its keys and buttons (task 1.5 groundwork). Physical pixels, DPI-aware.
That computer's regional settings use a comma as the decimal separator.

## A. Window layout

Window title "Calculator". Outer size 282 x 403; client size 264 x 331 (client origin 9,63 from the outer top-left).
Rectangles are left, top, width, height in client coordinates.

| Id | What | Rectangle |
|---|---|---|
| 160 | display frame | 13, 13, 238, 63 |
| 404 | expression line | 17, 16, 230, 18 |
| 401 | memory mark "M" | 13, 44, 17, 23 |
| 150 | main display | 35, 33, 211, 36 |
| 158 | display background | 35, 36, 208, 38 |
| - | button area | 13, 82, 238, 239 |

Buttons are 43 x 33; columns at x = 13, 62, 111, 160, 208; rows at y = 82, 122, 162, 203, 242, 282.

| Row | Buttons (id: meaning) |
|---|---|
| 1 | 122 MC, 123 MR, 124 MS, 125 M+, 126 M- |
| 2 | 83 backspace, 82 CE, 81 C, 80 plus/minus, 110 square root |
| 3 | 137 7, 138 8, 139 9, 91 divide, 118 percent |
| 4 | 134 4, 135 5, 136 6, 92 multiply, 114 reciprocal |
| 5 | 131 1, 132 2, 133 3, 94 minus, 121 equals (43 x 74, two rows) |
| 6 | 130 0 (92 x 33, two columns), 84 decimal point, 93 plus |

## B. Menus

View: Standard Alt+1 (dot), Scientific Alt+2, Programmer Alt+3, Statistics Alt+4 | History Ctrl+H, Digit grouping |
Basic Ctrl+F4 (dot), Unit conversion Ctrl+U, Date calculation Ctrl+E, Worksheets > (Mortgage, Vehicle lease,
Fuel economy (mpg), Fuel economy (L/100 km)).
Edit: Copy Ctrl+C, Paste Ctrl+V | History > (Copy history, Edit F2, Cancel edit Esc, Clear Ctrl+Shift+D).
Help: View Help F1 | About Calculator.

## C. Results

Escape before each line; [x] is a button click. Display and expression line exactly as shown (comma = decimal point).

| # | Keys | Display | Expression line | M |
|---|---|---|---|---|
| 1 | 2+3* | 5 | `2 + 3 *` | no |
| 2 | 2+3*4= | 20 | (empty) | no |
| 3 | 1/0= | Cannot divide by zero | `1 /` | no |
| 4 | 0/0= | Result is undefined | `0 /` | no |
| 5 | 1 [plus/minus] [square root] | Invalid input | `sqrt(-1)` | no |
| 6 | 5 [reciprocal] | 0,2 | `reciproc(5)` | no |
| 7 | 9 [square root] | 3 | `sqrt(9)` | no |
| 8 | 2+3% | 0,06 | `2 + 0,06` | no |
| 9 | 1/3= | 0,3333333333333333 | (empty) | no |
| 10 | 2/3= | 0,6666666666666667 | (empty) | no |
| 11 | 9999999999999999*9999999999999999= | 9,999999999999998e+31 | (empty) | no |
| 12 | 0.1+0.2= | 0,3 | (empty) | no |
| 13 | 5 [plus/minus] | -5 | (empty) | no |
| 14 | 123 [backspace] | 12 | (empty) | no |
| 15 | 2+= | 4 | (empty) | no |
| 16 | 2+3=== | 11 | (empty) | no |
| 17 | 100*5% | 5 | `100 * 5` | no |
| 18 | 5 [MS] [Escape] [MR] | 5 | (empty) | yes, also right after MS |
| 19 | 5 [M+] 3 [M+] [MR] (memory held 5 from line 18) | 13 | (empty) | yes |
| 20 | 12+3 [CE] 4= | 16 | (empty) | yes |
