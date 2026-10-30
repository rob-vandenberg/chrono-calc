# Windows 7 Calculator: Scientific view reference

Measured on 2026-10-30 on the owner's Windows computer, in the original Windows 7 Calculator (calc1.exe), through UI
Automation, posted keys and button clicks (task M2). Physical pixels at 100%; the decimal separator there is a comma.

## A. Window layout

Outer 526 x 403; client 508 x 331 (origin 9,63), the same height as Standard. Rectangles: left, top, width, height.

| Id | What | Rectangle |
|---|---|---|
| 160 | display frame | 13, 13, 482, 63 |
| 404 | expression line | 17, 16, 474, 18 |
| 150 | main display | 35, 33, 455, 36 |
| 401 | memory mark M | 13, 44, 17, 23 |
| 1079 | angle frame | 13, 82, 238, 33 |
| 321 / 322 / 323 | Degrees / Radians / Grads | 22,88,80,21 / 103,88,83,21 / 186,88,62,21 |
| 402 | bracket counter ("(=1"), a flat cell | 13, 122, 43, 33 |

Buttons 43 x 33; rows 82, 122, 162, 203, 242, 282; columns 13, 62, 111, 160, 208, 257, 306, 355, 403, 452.

| Row | Buttons (id label) |
|---|---|
| 82 | (angle frame) 122 MC, 123 MR, 124 MS, 125 M+, 126 M- |
| 122 | (counter) 146 Inv, 108 ln, 128 (, 129 ), 83 backspace, 82 CE, 81 C, 80 ±, 110 √ |
| 162 | 98 Int, 105 sinh, 102 sin, 111 x², 113 n!, 137 7, 138 8, 139 9, 91 /, 118 % (greyed) |
| 203 | 115 dms, 106 cosh, 103 cos, 97 x^y, 96 y√x, 134 4, 135 5, 136 6, 92 *, 114 1/x |
| 242 | 120 π, 107 tanh, 104 tan, 112 x³, 116 ∛x, 131 1, 132 2, 133 3, 94 -, 121 = (43 x 74) |
| 282 | 119 F-E, 127 Exp, 95 Mod, 109 log, 117 10^x, 130 0 (92 x 33), 84 decimal, 93 + |

Inv and F-E are toggles (amber face when on). There is no Hyp. With Inv on the labels change: ln to eˣ, Int to Frac,
dms to deg, π to 2*π, sin/cos/tan and sinh/cosh/tanh to their ⁻¹ forms; Inv turns off after such a function. x^y, log,
x³ and the other buttons have no inverse form: Inv stays on and they do their normal job. C and CE turn Inv off.
C turns F-E off, CE keeps it. % is greyed and does nothing.

## B. Switching views

A view change clears the number and the expression line and keeps the memory. The Scientific view starts in Degrees
every time (the angle unit does not survive a switch). Whether calc1 remembers the view after a restart was not tested.

## C. Keys (Scientific)

F3 Degrees, F4 Radians, F5 Grads; I Inv; S sin, O cos, T tan; Ctrl+S/O/T sinh/cosh/tanh; N ln, L log, M dms, P π,
V F-E, X and E Exp, Y x^y, Q x², D Mod, R 1/x; ( and ); # x³, @ √, ! n!, ; Int; Ctrl+Y y√x, Ctrl+G 10^x, Ctrl+B ∛x;
memory, F9, Delete, Backspace, Escape, Enter as in Standard. Letters only without Shift; H and % do nothing.

## D. Results

All lines are in `tests/test_reference_scientific.cpp` with the display, expression line, bracket counter and Inv that
Windows 7 showed (expression names: sind, asind, sinr, sing, sinh, asinh, ln, powe, log, powten, sqr, cube, cuberoot,
fact, dms, degrees, Int, frac, sqrt, reciproc; "2 ^", "2 yroot", "2 Mod"; brackets tight: "(2 +", "((").
32 digits at most; n! of non-integers is the gamma function; 171 n! = 1,2410180702176678234248405241031e+309.

## E. Look

1. Function buttons (also backspace, CE, C, ±, √, operators, % and = in Standard): face #F0F6FB / #D4E0ED / #D7E2F0,
   border #8797AA. Digit buttons and the decimal point: #F7FBFF / #E7EEF6 / #EDF5FB. Memory buttons: flat #D4E0ED with
   #F0F4F9 on the first line. = runs #F0F6FB, #ECF3F9, #D5E0ED, #D6E1EE, #D9E4F1 down its height.
2. Inv or F-E on: border #A09974, face #F0E5CE / #E2D0AE / #E2E9DF. A greyed button: border #B4C2D1, flat #D9E4F1.
3. Angle frame: border #8696AA, inside #D9E4F1; radio labels #1E395B; circles white with a #333333 dot.
4. Bracket counter: flat #D9E4F1, border #8E9CAD, text #1E395B centred.
5. Fonts: function labels, radio labels and the counter Segoe UI 13 px; digits Segoe UI 20 px (as Standard).

## F. Keys while an error shows (both views)

Every key and button is ignored except Escape, C and CE, which clear the error to 0; nothing is greyed. MS, MR and MC
are ignored too (nothing is stored or recalled); Inv, the angle units, ( and F-E are not taken; the expression line
stays ("5 + reciproc(0)" after a further =). With F-E on, 1/0= shows "1,e+0 /". Windows 7 has a fault here: Escape after
an error unticks F-E but keeps it on inside (2+3= then shows 5,e+0); we do not copy it: C turns F-E off, box and
numbers alike. Lines 50 to 54 of `tests/test_reference_scientific.cpp`.
