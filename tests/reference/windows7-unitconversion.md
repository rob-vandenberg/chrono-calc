# Windows 7 Calculator: Unit conversion reference

Measured on 2026-10-30 on the owner's Windows computer, in the original Windows 7 Calculator (calc1.exe), through real
mouse clicks and keys (task M6). Physical pixels at 100%. Regional settings there: en-NL, decimal symbol ',', digit
grouping symbol '.'. The combos and edit boxes are drawn by that computer's Windows 10 visual style. Lines marked
"Chrono Calc" say where we differ, and why (D-025).

## A. Layout and look

1. View > Unit conversion (Ctrl+U) shows the panel beside the calculator, in every view (never greyed), also with
   History shown. Client sizes with the panel: Standard 731 x 331, Scientific 975 x 331, Programmer 975 x 413,
   Statistics 731 x 456, Standard with History 731 x 456. The panel is 454 wide, 13 px right of the calculator part:
   Standard 264, 13, 454, 303; Scientific 508, 13, 454, 303; Programmer 508, 13, 454, 386; Statistics 264, 13, 454, 427;
   Standard with History 264, 13, 454, 428 (Chrono Calc: Scientific with History the same height, not measured).
2. Inside (Standard, client coordinates): the label "Select the type of unit you want to convert" 276, 26, 435, 30; the
   type list 276, 66, 432, 25; "From" 276, 105, 178, 17; the From box 276, 129, 432, 28; the From unit list 276, 162,
   432, 25; "To" 276, 206, 178, 17; the To box (read-only) 276, 231, 432, 28; the To unit list 276, 263, 432, 25. The
   lists are drop-down lists (not editable), rows of 19 px. No line inside the panel, only its frame.
3. Colours: the panel's frame #889AAD (1 px), its inside flat white #FFFFFF. Labels #1E395B, Segoe UI 13 px ("From"
   9 px capitals). Lists: text #000000, edge #ADADAD, face #E1E1E1, arrow #565656. From box: white, text #000000, edge
   #7A7A7A (#0078D7 with the focus), Segoe UI 15 px (digits 12 px high). To box: #F0F0F0, edge #7A7A7A, text #000000.
4. View menu: Basic (Ctrl+F4), Unit conversion (Ctrl+U), Date calculation (Ctrl+E) carry one dot among them;
   Worksheets > Mortgage, Vehicle lease, Fuel economy (mpg), Fuel economy (L/100 km), without shortcuts. None is greyed in
   any view. Ctrl+U while the panel shows keeps it (no toggle); Ctrl+F4 hides it (282 x 403 outside again).

## B. The lists

1. Unit types, in this order: Angle, Area, Energy, Length, Power, Pressure, Temperature, Time, Velocity, Volume,
   Weight/Mass. The first time: Angle, with Degree in both unit lists and both boxes empty.
2. The To list is the same list as From. Choosing a type sets both lists to its first unit and empties both boxes,
   every time (nothing is remembered per type).
3. The units, in order:
   1. Angle: Degree, Gradian, Radian
   2. Area: Acres, Hectares, Square centimeter, Square feet, Square inch, Square kilometer, Square meters, Square mile,
      Square millimeter, Square Yard
   3. Energy: British Thermal Unit, Calorie, Electron-Volts, Foot-Pound, Joule, Kilocalorie, Kilojoule
   4. Length: Angstrom, Centimeters, Chain, Fathom, Feet, Hand, Inch, Kilometers, Link, Meter, Microns, Mile,
      Millimeters, Nanometer, Nautical Miles, PICA, Rods, Span, Yard
   5. Power: BTU/minute, Foot-Pound/minute, Horsepower, Kilowatt, Watt
   6. Pressure: Atmosphere, Bar, Kilo Pascal, Millimeter of mercury, Pascal, Pound per square inch (PSI)
   7. Temperature: Degrees Celsius, Degrees Fahrenheit, Kelvin
   8. Time: Day, Hour, Microsecond, Millisecond, Minute, Second, Week
   9. Velocity: Centimeter per second, Feet per second, Kilometer per hour, Knots, Mach (at std. atm), Meter per second,
      Miles per hour
   10. Volume: Cubic centimeter, Cubic feet, Cubic inch, Cubic meter, Cubic yard, Fluid ounce (UK), Fluid ounce (US),
       Gallon (UK), Gallon (US), Liter, Pint (UK), Pint (US), Quart (UK), Quart (US)
   11. Weight/Mass: Carat, Centigram, Decigram, Dekagram, Gram, Hectogram, Kilogram, Long ton, Milligram, Ounce, Pound,
       Short ton, Stone, Tonne

## C. Results

The To box as Windows 7 shows it. Part C1: From = 1 of the first unit, To = each unit in list order.

| # | Type | To, for every unit of the list |
|---|---|---|
| 1 | Angle | 1 \| 1,111111111111111 \| 0,0174532925199433 |
| 2 | Area | 1 \| 0,40468564224 \| 40468564,224 \| 43560 \| 6272640 \| 0,0040468564224 \| 4046,8564224 \| 0,0015625 \| 4046856422,4 \| 4840 |
| 3 | Energy | 1 \| 251,9957963122194 \| 6,585142025517001e+21 \| 778,1693709678747 \| 1055,056 \| 0,2519957963122194 \| 1,055056 |
| 4 | Length | 1 \| 0,00000001 \| 4,970969537898672e-12 \| 5,468066491688539e-11 \| 3,280839895013123e-10 \| 9,84251968503937e-10 \| 3,937007874015748e-9 \| 0,0000000000001 \| 4,970969537898672e-10 \| 0,0000000001 \| 0,0001 \| 6,21371192237334e-14 \| 0,0000001 \| 0,1 \| 5,399568034557235e-14 \| 2,371063015836614e-8 \| 1,988387815159469e-11 \| 4,374453193350831e-10 \| 1,093613298337708e-10 |
| 5 | Power | 1 \| 778,1693709678747 \| 0,0235808900293295 \| 0,0175842666666667 \| 17,58426666666667 |
| 6 | Pressure | 1 \| 1,01325 \| 101,325 \| 760,1275318829707 \| 101325 \| 14,69594940039221 |
| 7 | Temperature | 1 \| 33,8 \| 274,15 |
| 8 | Time | 1 \| 24 \| 86400000000 \| 86400000 \| 1440 \| 86400 \| 0,1428571428571429 |
| 9 | Velocity | 1 \| 0,0328083989501312 \| 0,036 \| 0,019438444924406 \| 2,938641460175678e-5 \| 0,01 \| 0,022369362920544 |
| 10 | Volume | 1 \| 3,531466672148859e-5 \| 0,0610237440947323 \| 0,000001 \| 1,307950619314392e-6 \| 0,035195079727854 \| 0,033814022701843 \| 2,199692482990878e-4 \| 2,641720523581484e-4 \| 0,001 \| 0,0017597539863927 \| 0,0021133764188652 \| 8,798769931963512e-4 \| 0,0010566882094326 |
| 11 | Weight/Mass | 1 \| 20 \| 2 \| 0,02 \| 0,2 \| 0,002 \| 0,0002 \| 1,968413055222121e-7 \| 200 \| 0,0070547923899161 \| 4,409245243697552e-4 \| 2,204622621848776e-7 \| 3,149460888355394e-5 \| 0,0000002 |

Chrono Calc differs where Windows 7's rounded factors give wrong digits (D-025):
1. Energy and Power: every cell after the first, because the IT BTU is exactly 4.1868 J/g * 453.59237 g * 5/9 =
   1055.05585262 J (Windows 7: 1055.056). Energy: 1 | 251,9957611111111 | 6,585140678190667e+21 | 778,1692622659649 |
   1055,05585262 | 0,2519957611111111 | 1,05505585262. Power: 1 | 778,1692622659649 | 0,0235808867353323 |
   0,0175842642103333 | 17,58426421033333 (found in review, before the first push).
2. With it the exact SI electronvolt (1.602176634e-19 J; Windows 7 used the CODATA 2002 value 1.60217653e-19) and the
   exact foot-pound (0.3048 * 0.45359237 * 9.80665 = 1.3558179483314004 J; Windows 7 cut it to 1.3558179483314).
3. Pressure: Millimeter of mercury 760 (1 atmosphere is 760 mmHg by definition; Windows 7 used 133.3 Pa); Pound per
   square inch 14,69594877551345 (0.45359237 * 9.80665 / 0.0254² Pa; Windows 7 used 6894.757 Pa).

Part C2, other results:

| # | From | To | Windows 7 |
|---|---|---|---|
| 1 | 0 Degrees Celsius | Degrees Fahrenheit | 32 |
| 2 | 0 Degrees Celsius | Kelvin | 273,15 |
| 3 | 100 Degrees Fahrenheit | Degrees Celsius | 37,77777777777778 |
| 4 | 100 Degrees Fahrenheit | Kelvin | 310,9277777777778 |
| 5 | -40 Degrees Celsius | Degrees Fahrenheit | -40 |
| 6 | 1 Kelvin | Degrees Celsius | -272,15 |
| 7 | 0 Kelvin | Degrees Fahrenheit | -459,67 |
| 8 | -500 Degrees Celsius | Kelvin | -226,85 (no error below absolute zero) |
| 9 | 1 Inch | Centimeters | 2,54 |
| 10 | 1 Mile | Kilometers | 1,609344 |
| 11 | 1 Nautical Miles | Meter | 1852 |
| 12 | 3 Feet | Yard | 1 |
| 13 | 1 Angstrom | Meter | 0,0000000001 |
| 14 | 123456789 Kilometers | Inch | 4860503503937,008 |
| 15 | 1e30 Angstrom | Mile | 6,21371192237334e+16 |
| 16 | 1e30 Mile | Angstrom | 1,609344e+43 |
| 17 | 1e-30 Angstrom | Mile | 6,21371192237334e-44 |
| 18 | 1 Pound | Kilogram | 0,45359237 |
| 19 | 1 Ounce | Gram | 28,349523125 |
| 20 | 1 Carat | Gram | 0,2 |
| 21 | 1 Stone | Pound | 14 |
| 22 | 1 Short ton | Kilogram | 907,18474 |
| 23 | 1 Long ton | Kilogram | 1016,0469088 |
| 24 | 1 Gallon (US) | Liter | 3,785411784 |
| 25 | 1 Gallon (UK) | Liter | 4,54609 |
| 26 | 1 Cubic feet | Liter | 28,316846592 |
| 27 | 1 Week | Second | 604800 |
| 28 | 1 Day | Microsecond | 86400000000 |
| 29 | 180 Degree | Radian | 3,141592653589793 |
| 30 | 180 Degree | Gradian | 200 |
| 31 | 1 Radian | Degree | 57,29577951308233 (Chrono Calc: 57,29577951308232; 180/π is 57,295779513082320877, so Windows 7's last digit is wrong) |
| 32 | 1 Calorie | Joule | 4,1868 |
| 33 | 1 Horsepower | Watt | 745,6998715822702 |
| 34 | 1 Atmosphere | Pascal | 101325 |
| 35 | 1 Knots | Kilometer per hour | 1,852 |
| 36 | 1 Acres | Square meters | 4046,8564224 |
| 37 | 1 Hectares | Acres | 2,471053814671653 |
| 38 | 1 Feet | Yard | 0,3333333333333333 |
| 39 | 1 Yard | Feet | 3 |
| 40 | 1,e+30 Kilometers | Inch | 3,937007874015748e+34 |
| 41 | 1 Inch, then To Chain | Chain | 0,0012626262626263 |
| 42 | 1 Kilometers | Chain | 49,70969537898672 |
| 43 | 5 Inch | Centimeters | 12,7 |

1. At most 16 significant digits; large and small numbers as "6,21371192237334e+16" (the engine's own way of writing a
   number). Not measured: a result with one digit before the exponent; Chrono Calc writes it as the engine and the
   display do ("1,e+32").
2. The To box is never grouped, also with View > Digit grouping on (line 14 stays 4860503503937,008).

## D. Behaviour

1. The From box is its own edit box, not the display. Typing on the calculator (123, +, 2, =, CE, 1/0=) changes only the
   display; the boxes stay. Typing in From changes neither the display nor the calculator, and From never goes into the
   display. The To box follows every character typed (1 → 2,54; 12 → 30,48; 123 → 312,42 for Inch → Centimeters).
2. What From takes (Meter → Centimeters): digits, the regional decimal symbol, + and -, and a small "e"; everything else
   is refused as it is typed ("abc" gives nothing, "12a3" gives 123, "1E30" gives 130, "1.5" gives 15, "1.234,5" gives
   1234,5, " 2 " gives 2). At most 25 characters. Valid: "-5" (-500), "+5" (500), "-0,5" (-50), "1,5" (150), "1e30",
   "1,e+30". Accepted as text but To empty: "5-", "--5", "1,5,5", and 17 or more digits.
3. With an invalid From, Windows 7 keeps the focus in From ("1 : this value is not valid") and a click on a list does
   not take effect. Chrono Calc: the To box is empty and nothing is held (D-025).
4. From has the standard edit menu (Undo, Cut, Copy, Paste, Delete, Select All, ...). F9, Enter and Escape in From do
   nothing; characters typed there never reach the display.
5. Changing the From or To unit keeps From and calculates To again at once. Changing the type: see part B line 2.
6. No swap button and no other control.
7. Keys: Tab goes From → From unit → To box → To unit → the calculator → the type list → From. Up and Down on a closed
   list change its unit at once. With a list focused, digits and the other keys go to the calculator (7 and 8 appear in
   the display; a letter does not search the list); Enter and Escape act on the calculator and the focus goes to it.
8. Negative values convert (-3 Meter is -300 Centimeters).
9. A view change keeps the panel, the type, the units and From (5 Inch → 12,7 Centimeters in all four views). Basic
   hides the panel; Ctrl+U shows it again with type, units and From as they were; so does Ctrl+E then Ctrl+U.
10. Remembered (HKCU\Software\Microsoft\Calc): UnitConv=1 while the panel shows (Templates and DateTime for the other
    two); nothing holds the type or the units. Chrono Calc remembers the panel (`window/sidePanel`), not the type.
    After a restart: not measured.
11. Copy: Ctrl+C with the focus on the calculator copies the display; in From it copies From's selected text; the To box
    can be selected and copied (12,7).

## G. How From is read (follow-up from chrono-calc (7), measured 2026-10-30)

Length, From Meter, To Centimeters; before each line From was 7 (To 700), then all of From selected and the text typed.

| # | From | To |
|---|---|---|
| 1 | 00000000000000001 | 100 |
| 2 | e5 | 1,e+7 |
| 3 | 1e | 1,e+2 |
| 4 | , | 0 |
| 5 | - | 0 |
| 6 | + | 0 |
| 7 | -0 | 0 |
| 8 | 0,0000000000000000001 | empty |
| 9 | 1e400 | 1,e+402 |
| 10 | 1e10000 | empty |

1. The same reading as the Worksheets (windows7-worksheets.md part E) except that a minus is allowed: zeros before the
   whole part's first other digit do not count, every digit after the decimal symbol does (16 at most); no digits is 0,
   or 1 before an exponent; an exponent without digits is 0; at most four exponent digits. A From typed with an
   exponent gives a To written with one (lines 2 and 3).
2. 123 with the 2 selected, then 'a' typed: From becomes 13 (the refused key still deletes the selection).
3. 25 characters in From, the cursor after the first, then 9 typed: refused (the edit limit is 25).
