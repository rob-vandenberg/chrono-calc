# Windows 7 Calculator: Statistics view reference

Measured on 2026-10-30 on the owner's Windows computer, in the original Windows 7 Calculator (calc1.exe), through UI
Automation, posted keys and button clicks (task M4). Physical pixels at 100%; the decimal separator there is a comma.

## A. Window layout

Outer 282 x 528; client 264 x 456 (origin 9,63): as narrow as Standard and 125 px taller. Rectangles: left, top, width,
height in client coordinates.

| Id | What | Rectangle |
|---|---|---|
| 186 | display frame (border #8E9CAD, inside #E5EEFB at the top to #FEFEFF near the bottom) | 13, 13, 238, 188 |
| 1051, 1052 | selection buttons ▲ and ▼: flat #E2EDFA, a filled triangle #1E395B 9 x 5 px (rows 1-3-5-7-9 px), 6 px from the left, 8 px from the top | 194, 21, 22, 21 and 221, 21, 22, 21 |
| 187 | list box (no scroll bar; 5 rows of 18 px: 17 px row and a 1 px dotted line #74AABD, every other pixel) | 21, 47, 224, 91 |
| 182, 181 | "Count = " and the count (Segoe UI 12 px, #1E395B); "Count = 0" at the start | 19, 143, 47, 19 and 66, 143, 66, 19 |
| 150 | number (Consolas 30 px) | 35, 159, 208, 36 |
| 401 | M (Consolas 18 px) | 19, 171, 17, 23 |

There is no expression line. A thin line #D1D7DE separates the list from the count and the number. List items are
right-aligned (ending at x 243), grey #596B72, a fixed-width font with digits 10 px high; the selected row is #9ECBF5;
the list's background is the display's own face.

Buttons 43 x 33; rows 207, 247, 287, 328, 367, 407; columns 13, 62, 111, 160, 208.

| Row | Buttons |
|---|---|
| 207 | MC, MR, MS, M+, M- (memory face) |
| 247 | backspace, CAD, C, F-E (a toggle), Exp |
| 287 | 7, 8, 9, x̄, x̄² |
| 328 | 4, 5, 6, Σx, Σx² |
| 367 | 1, 2, 3, σn, σn-1 |
| 407 | 0 (92 x 33), decimal point, ±, Add |

No CE, no Inv, no operators; nothing is greyed. Faces: digits and the decimal point the digit face; the others the
function face; the memory row flat. Label fonts: digits Segoe UI 20 px; MC to M-, CAD, C, F-E, ±, Exp, Add and the
decimal point Segoe UI 13 px; backspace Segoe UI Symbol 30 px; x̄ to σn-1 Segoe UI Symbol 17 px.

## B. Menus

The dot is on View > Statistics (Alt+4) and on Basic. View > History is greyed; Digit grouping is not.

## C. Keys (Statistics)

Enter = Add; A = x̄, Ctrl+A = x̄²; S = Σx, Ctrl+S = Σx²; T = σn, Ctrl+T = σn-1; D = CAD; Escape = C; Backspace; F9 = ±;
X and E = Exp; V = F-E; Ctrl+M/R/P/Q/L the memory. Nothing: Insert, Ctrl+D, Ctrl+Shift+A/S/D/T, Delete (while the
window has the keyboard focus), + - * / = @ # % ; ! ( ), and the letters Q R N L M P O Y I C G B U W Z F H J K.

## D. Behaviour

1. Add puts the number shown at the end of the list, selects it, and shows 0; Count follows. Add right after Add adds 0.
   Add right after a statistic adds the result. The item is the number as the display writes it ('5,' gives '5', '0,10'
   gives '0,1'; 9 Exp 300 gives '9,e+300'); 16 digits at most are typed.
2. A statistic shows its result as the number; nothing goes to the list; there is no expression line. ± after it changes
   only the number. Too few values (none; one for σn-1) show the error "Enter data to calculate"; σn of one value is 0.
3. During that error digits, Add, the statistics, ±, MS, MR, backspace, F-E and Exp are ignored; C gives 0 and keeps the
   list, CAD gives 0 and clears the list.
4. A click on an item selects it and gives the list the keyboard focus; the number does not change (a double click
   neither). With the list focused: the arrow keys move the selection (no wrap at the ends), Home and End go to the first
   and last item, Page Up and Page Down move by a page, Delete removes the selected item and the item that moves up into
   its place is selected; digits still go to the number; Enter (Add) and Escape (C) give the focus back to the window, a
   click on a calculator button too (Delete then does nothing). ▲ and ▼ move the selection by one, no wrap.
5. Windows 7 fault, not copied: Delete never removes the last item of the list (also not the only one), so the bottom
   value can only go with CAD. Chrono Calc removes it and selects the new last item.
6. C (Escape) clears only the number; CAD (D) clears the number and the list; the memory stays with both.
7. A view change clears the number, keeps the list and the memory, and turns F-E off (in every view). F-E on draws the
   items in the same notation ('5,e+0'); it survives C and CAD.
8. The list scrolls only to keep the selection in view: with 30 items the newest is at the bottom, selected.
9. The list is not kept after closing and opening the calculator (checked by the owner by hand); Chrono Calc does not keep it either.

## E. Results

Precision is 16 digits, as Standard. In `tests/test_reference_statistics.cpp`:

| List | x̄ | x̄² | Σx | Σx² | σn | σn-1 |
|---|---|---|---|---|---|---|
| 5, 3, 4 | 4 | 16,66666666666667 | 12 | 50 | 0,816496580927726 | 1 |
| 1, 2, 3, 4 | 2,5 | 7,5 | 10 | 30 | 1,118033988749895 | 1,290994448735806 |
| 2, 4, 4, 4, 5, 5, 7, 9 | 5 | 29 | 40 | 232 | 2 | 2,138089935299395 |
| 1, 2 | | | | | 0,5 | 0,7071067811865475 |
| 1, 2, 3 | | | | | 0,816496580927726 | 1 |
| 1, 1, 1 | | | | | 0 | |
| 7 | | | | | 0 | Enter data to calculate |
| -5, 2,5 | -1,25 | | | | | |
| 0,1, 0,2 | | | 0,3 | | | |
| 9e300, 9e300 | | | 1,8e+301 | 1,62e+602 | | |
| (empty) | Enter data to calculate | the same | the same | the same | the same | the same |

Also: 5, 3, 4, then σn, Add: the list holds 0,816496580927726 too, Count 4; then x̄² is 12,66666666666667.
