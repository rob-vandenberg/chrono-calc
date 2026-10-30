# Windows 7 Calculator: History, Digit grouping, Copy and Paste reference

Measured on 2026-10-30 on the owner's Windows computer, in the original Windows 7 Calculator (calc1.exe), through UI
Automation, posted keys and button clicks (task M5). Physical pixels at 100%. Regional settings there: en-NL, decimal
symbol ',', digit grouping symbol '.', grouping 3;0. Lines marked "Chrono Calc" say where we differ, and why.

## A. History layout

1. View > History (Ctrl+H) is a tick item; Ctrl+H turns it on and off. It works in Standard and Scientific; in Programmer
   and Statistics the item is greyed (it keeps its tick) and Ctrl+H does nothing. One setting for both views.
2. Standard with History shown: client 264 x 456 (hidden 264 x 331), 125 px taller. Rectangles in client coordinates:
   display frame 13, 13, 238, 188; ▲ 197, 20, 22, 21; ▼ 224, 20, 22, 21; history list 20, 46, 224, 91 (five rows, no
   scroll bar); a thin line #D1D7DE at y 138; expression line 17, 141, 230, 18; number 35, 158, 211, 36; M 13, 169, 17,
   23. Every button is 125 px lower (MC at 13, 207).
3. Scientific with History shown: client 508 x 456; display frame 13, 13, 482, 188; ▲ 441, 20; ▼ 468, 20; list 20, 46,
   468, 91; expression line 17, 141, 474, 18; number 35, 158, 455, 36; M 13, 169. Everything below is 125 px lower: the
   angle frame 13, 207, 238, 33, Degrees 22, 213, 80, 21, the bracket counter 13, 247, 43, 33, Inv 62, 247, MC 257, 207.
4. The list looks like the Statistics list: one line per entry, the expression only (no result), right-aligned, grey
   #596B72, fixed-width digits 10 px high, rows of 18 px with a dotted line #74AABD, the selected row #9ECBF5; the newest
   at the bottom, selected and scrolled into view.
5. Showing or hiding slides the window's height in about 150 ms (200 ms in all); the buttons do not slide, they jump.
   Chrono Calc: the window takes its new height at once (D-024).

## B. What History records

The entry is the expression as the engine writes it; the expression line is never part of it.

| # | View | Keys | Entry | Display |
|---|---|---|---|---|
| 1 | Standard | 2+3= | `2 + 3` | 5 |
| 2 | Standard | 2+3*4= | `2 + 3 * 4` | 20 |
| 3 | Standard | 5= | `5` | 5 |
| 4 | Standard | 5+= | `5 + 5` | 10 |
| 5 | Standard | 2+3== | `2 + 3`, then `5 + 3` | 8 |
| 6 | Standard | 9 [sqrt] | nothing yet; `sqrt(9)` once a digit or CE follows | 3 |
| 7 | Standard | 9 [sqrt] +1= | `sqrt(9) + 1` | 4 |
| 8 | Standard | 100+5%= | `100 + 5` | 105 |
| 9 | Standard | 1/0= | nothing | Cannot divide by zero |
| 10 | Standard | 2 F9 +3= | `-2 + 3` | 1 |
| 11 | Standard | 7 MS, 2+MR= | `2 + 7` | 9 |
| 12 | Standard | 1/3= | `1 / 3` | 0,3333333333333333 |
| 13 | Scientific | 2+3*4= | `2 + 3 * 4` | 14 |
| 14 | Scientific | 30 sin +1= | `sind(30) + 1` | 1,5 |
| 15 | Scientific | (2+3)*4= | `(2 + 3) * 4` | 20 |
| 16 | Scientific | 2 [x^y] 10= | `2 ^ 10` | 1024 |
| 17 | Scientific | 5 [n!] = | `fact(5)` | 120 |
| 18 | Standard | 2+3 Escape | nothing | 0 |

1. Line 6: 4 [sqrt] [sqrt] then CE gives `sqrt(sqrt(4))`. The C button does not add it; the Escape key does. Chrono Calc:
   Escape is C and adds nothing, as the button (D-024). With History hidden: 9 [sqrt] then 5 shows 5 with an empty
   expression line; then CE shows 0 with an empty line; 2+9 [sqrt] then 5 shows 5 with `2 +`.
2. One history per view: Standard and Scientific keep their own; both are kept over every view change. 150 entries were
   all kept (no limit found). C, CE, Escape and a view change keep the history; only Edit > History > Clear empties it.
   History is recorded while it is hidden too.
3. After a restart: not measured (the owner's computer runs calc1 in a container there).

## C. Using History

1. A click on an entry selects it, shows its result and gives the list the keyboard focus; the expression line is kept
   as it was (2+ then a click on `1 / 3`: 0,3333333333333333 with `2 +`; = gives 2,333333333333333 and the entry
   `2 + 0,3333333333333333`). A double click also opens Edit on it.
2. The result shown is the 16-digit (Scientific 32-digit) value as written, not the exact one: choose `1 / 3`, *3= gives
   0,9999999999999999 and the entry `0,3333333333333333 * 3`. After choosing `2 + 3` (5), +1= gives the entry `5 + 1`.
3. With the keyboard focus in the window and History shown with entries: Up, Down, Page Up and Page Down move the
   selection by one and show that entry's result; Home, End and Delete do nothing. With History hidden, or shown but
   empty, they do nothing.
4. The ▲ and ▼ buttons move the selection by one and show the entry's result, no wrap at the ends; the keyboard focus
   stays in the window.
5. With the focus in the list: Up, Down, Page Up and Page Down move by one, show the result and give the focus back to
   the window; Home and End go to the first and last entry and keep the focus; Enter is = (and gives the focus back);
   Escape is C; Delete does nothing.
6. While an error shows: a click or Up clears it and shows the entry's result; F2 opens the edit box and the error stays;
   Ctrl+Shift+D empties the list and the error stays.
7. Edit > History > Copy history puts every entry on the clipboard, each followed by a line end: `2 + 3\r\n2 * 4\r\n`.
   With an empty history the clipboard is left as it is. Chrono Calc ends lines with `\n`, the line end of Linux (D-024).
8. Edit > History > Clear (Ctrl+Shift+D) empties the list only; the number stays.
9. The Edit > History items are never greyed. In Programmer the History submenu is absent. Chrono Calc: it is there,
   greyed (D-024).

## D. History Edit (F2)

1. F2 edits the selected entry (the newest unless another was chosen) in an edit box over its row (Standard 21, 82, 223,
   18 for the third row: the list's left + 1, the row's top), right-aligned, no frame, the caret at the start. With an
   empty history F2 does nothing. While editing, Edit > History > Edit reads "Recalculate" with the key "=".
2. Enter (or = typed in the box) with a valid text calculates it in place: the entry gets the written form, the display
   the result, the expression line stays empty, the entry stays selected and the list gets the focus. No entry is added.
   An invalid text, or one that ends in an error, changes nothing and the box stays open. Escape or Cancel edit drop the
   edit. A view change while editing applies the edit.

| # | View | Typed | Accepted | Entry | Display |
|---|---|---|---|---|---|
| 1 | Standard | `2+3*4` | yes | `2 + 3 * 4` | 20 |
| 2 | Standard | `(2+3)*4` | yes | `(2 + 3) * 4` | 20 |
| 3 | Standard | `sqrt(9)+1` | yes | `sqrt(9) + 1` | 4 |
| 4 | Standard | `sqrt 9` | no | | |
| 5 | Standard | `1,5+2` | yes | `1,5 + 2` | 3,5 |
| 6 | Standard | `1.5+2` | yes (the '.' is dropped) | `15 + 2` | 17 |
| 7 | Standard | `-2+3` | yes | `-2 + 3` | 1 |
| 8 | Standard | `2+-3` | yes | `2 + -3` | -1 |
| 9 | Standard | `7` | yes | `7` | 7 |
| 10 | Standard | (empty) | no | | |
| 11 | Standard | `  2  +  3  ` | yes | `2 + 3` | 5 |
| 12 | Standard | `1/0` | no (no error shown) | | |
| 13 | Standard | `reciproc(4)` | yes | `reciproc(4)` | 0,25 |
| 14 | Standard | `negate(2)+1` | yes | `negate(2) + 1` | -1 |
| 15 | Standard | `2^10` | yes | `2 ^ 10` | 1024 |
| 16 | Standard | `fact(5)` | yes | `fact(5)` | 120 |
| 17 | Standard | `50%`, `100+5%` | no | | |
| 18 | Standard | `12345678901234567890+1` | no | | |
| 19 | Scientific | `sind(30)+1` | yes | `sind(30) + 1` | 1,5 |
| 20 | Scientific | `2^10`, `2 ^ 10` | yes | `2 ^ 10` | 1024 |
| 21 | Scientific | `fact(5)` | yes | `fact(5)` | 120 |
| 22 | Scientific | `10 Mod 3` | yes | `10 Mod 3` | 1 |
| 23 | Scientific | `1,e+5` | yes | `100000` | 100000 |
| 24 | Scientific | `2 yroot 8` | yes | `2 yroot 8` | 1,0905077326652576592070106557607 |
| 25 | Scientific | `sqr(3)`, `cube(2)`, `powten(2)` | yes | | 9, 8, 100 |
| 26 | Scientific | `sin(30)`, `(2+3`, `2+3)`, `5!`, `pi` | no | | |

3. Editing the middle one of three entries (`2 * 4` to `6*7`) gives `6 * 7` and 42 there; the selection stays on it.
4. While the edit box is open, a mouse click on a calculator button types its character into the box (8 gives `6*78`);
   nothing is calculated. View > History (Ctrl+H) while editing drops the edit; a view change applies it.

## E. Digit grouping (View > Digit grouping)

1. A tick item without a key; off at the start. Turning it on or off redraws the number at once.
2. Standard: typing 1234567 shows 1, 12, 123, 1.234, 12.345, 123.456, 1.234.567; 1234567,891 is 1.234.567,891;
   -1234567 is -1.234.567; 1234567*1000= is 1.234.567.000; 1/3= is 0,3333333333333333 (decimals not grouped);
   10000000000*10000000000= is 1,e+20; 9999999999999999+1= is 1,e+16; error texts are unchanged.
3. The expression line and the history entries are never grouped: 1234567+ shows `1234567 +`; the entry `1234567 + 1`.
4. Scientific: 123456789 is 123.456.789; 2/3= is 0,66666666666666666666666666666667; F-E on, typing 1234567 shows
   1.234.567; 1234567 Exp 5 shows 1.234.567,e+5 and = gives 123.456.700.000.
5. Programmer: Hex FFFFFFFF is `FFFF FFFF`; Dec 1234567 is 1.234.567 and -1234567 is -1.234.567; Oct 1234567 is
   `1 234 567`; Bin 11111111111 is `111 1111 1111`. The bit display does not change.
6. Statistics: the number and the list's values are grouped (1.234.567).

## F. Copy and Paste

1. Edit > Copy, Ctrl+C and Ctrl+Insert copy (Shift+Insert pastes) the number exactly as shown, never the expression line: 1234567,5 (with
   grouping 1.234.567,5); an error copies its text; with `2 +` and 3 shown: 3; F-E 5: 5,e+0; Hex FF: FF; Hex FFFFFFFF
   with grouping: FFFF FFFF; Bin 101: 101; Bin 111111111 with grouping: 1 1111 1111. Copy is never greyed.
2. Statistics' Copy dataset: every value with a line end (`5\r\n3\r\n4\r\n`); an empty list leaves the clipboard as is.
3. Paste reads the text as typed keys of the current view; a character without a key is skipped. Paste is greyed when
   the clipboard holds no text. While an error shows, Paste changes nothing.

| # | View | Pasted | Display | Expression line |
|---|---|---|---|---|
| 1 | Standard | `123` | 123 | |
| 2 | Standard | `1,5` | 1,5 | |
| 3 | Standard | `1.5` | 15 | |
| 4 | Standard | `1.234,5` | 1234,5 | |
| 5 | Standard | `1 234` | 1234 | |
| 6 | Standard | `-5` | -5 | |
| 7 | Standard | ` 5 ` | 5 | |
| 8 | Standard | `5-` | 5 | `5 -` |
| 9 | Standard | `abc` | 0 | |
| 10 | Standard | `12a3` | 123 | |
| 11 | Standard | `2+3=` | 5 (entry `2 + 3`) | |
| 12 | Standard | `2+3` | 3 | `2 +` |
| 13 | Standard | `2*3=4=` | 12 (entries `2 * 3`, `4 * 3`) | |
| 14 | Standard | `12345678901234567890` | 1234567890123456 | |
| 15 | Standard | `1e5` | 15 | |
| 16 | Standard | `1,2e+3` | 1,23 | |
| 17 | Standard | `5,e+3` | 5,3 | |
| 18 | Standard | `(2+3)*4=` | 20 (entry `2 + 3 * 4`) | |
| 19 | Standard | `2\r\n3` | 3 (no entry) | |
| 20 | Standard | 200 digits | 1111111111111111 | |
| 21 | Standard | after 7+: `5`, then = | 12 | |
| 22 | Standard | `:m` after 4 | 4, M shown (MS) | |
| 23 | Standard | `:r`, `:p`, `:q`, `:c` | MR, M+, M-, MC | |
| 24 | Standard | `:e`, `\` | nothing | |
| 25 | Standard | `@` after 4 | 2 | `sqrt(4)` |
| 26 | Standard | `r` after 4 | 0,25 | `reciproc(4)` |
| 27 | Standard | `q` after 4 | 0 (as C) | |
| 28 | Standard | `s` after 4 | 4 | |
| 29 | Standard | `5%` after 100+ | 5 | `100 + 5` |
| 30 | Scientific | `(2+3)*4=`, `2+3*4=` | 20, 14 | |
| 31 | Scientific | `s` after 30 | 0,5 | `sind(30)` |
| 32 | Programmer | Hex `ff`, `FF` | FF | |
| 33 | Programmer | Hex `1G`; Bin `102`; Dec `1,5` | 1; 10; 15 | |
| 34 | Statistics | `1,5` | 1,5 (not added) | |
| 35 | Statistics | `1\r\n2` | 2 (1 added) | |
| 36 | Statistics | `1\r\n2\r\n` | 0 (1 and 2 added) | |
| 37 | Scientific, Programmer | `q` after 4 | 0 (as C, not x²) | |
| 38 | Standard | `2+-3=` | -1 (entry `2 - 3`) | |
| 39 | Scientific | `1e-5` | 1,e-5 | |

4. Rules read from these lines: the regional decimal symbol is the decimal point and the other one is skipped; a minus
   at the start is the number's sign; `e` followed by `+` or `-` is skipped with that sign where the view has no Exp; a
   line end ends the number being typed, and in Statistics it is Add; `:m :r :p :q :c` are MS MR M+ M- MC; `q` is C in
   every view; 16 digits at most are taken, as typed.

## G. Statistics Edit menu

1. Edit: Copy, Paste, Copy dataset, Edit (F2), Cancel edit (Esc), Clear (D); none greyed. Clear is CAD.
2. F2 edits the selected value in an edit box over its row (22, 83, 223, 18 for the third row), holding the value.
   Enter with a number replaces it (`7`, `1,5`); the order and the selection stay; the list gets the focus. Enter with an
   empty text removes the value. Escape drops the edit. While editing, the Edit item is gone from the menu.
3. Enter with a text that is not a number (`abc`) removes the value too. Chrono Calc: the edit box stays open, as History's
   does, so a typing mistake never throws a value away (D-024). Chrono Calc greys the Edit item while editing instead of
   removing it (D-024).

## H. Remembered settings

1. HKCU\Software\Microsoft\Calc holds `Window_Placement`, `layout` (the view), `ShowHistory`, and `UseSep` (Digit
   grouping, written when it is first turned on or off). Chrono Calc remembers the same: the position, the view,
   History shown and Digit grouping (`~/.config/chrono-calc/chrono-calc.conf`). Not measured: a restart of calc1.
