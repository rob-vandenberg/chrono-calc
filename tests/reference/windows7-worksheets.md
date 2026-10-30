# Windows 7 Calculator: Worksheets reference

Measured on 2026-10-30 on the owner's Windows computer, in the original Windows 7 Calculator (calc1.exe), through real
mouse clicks and keys (task M8): list items chosen by a click on the list and Home, Down, Enter; values typed after a
click into the box and select-all; Calculate a real click; every value read back from its box before the result was
read. Physical pixels at 100%. Regional settings there: en-NL, decimal symbol ','. The controls are drawn by that
computer's Windows 10 visual style. Lines marked "Chrono Calc" say where we differ, and why (D-027).

## A. Layout and look

1. View > Worksheets: "&Mortgage", "&Vehicle lease", "&Fuel economy (mpg)", "F&uel economy (L/100 km)", in this order,
   without shortcuts. The items have no dot of their own; while a worksheet shows, Basic, Unit conversion and Date
   calculation have none either.
2. The worksheet shows in the frame of Unit conversion and Date calculation (windows7-unitconversion.md part A line 1),
   in every view (outer / client / panel): Standard 749 x 403 / 731 x 331 / 264, 13, 454, 303; Scientific 993 x 403 /
   975 x 331 / 508, 13, 454, 303; Programmer 993 x 485 / 975 x 413 / 508, 13, 454, 386; Statistics 749 x 528 / 731 x 456 /
   264, 13, 454, 427; Standard with History 749 x 528 / 731 x 456 / 264, 13, 454, 428.
3. Inside (Standard, client coordinates): the label "Select the value you want to calculate" 276, 24, 430, 35; the list
   (a drop-down list, sorted) 276, 54, 430, 25; the value labels, each 213 x 21, at 276 and 93, 124, 156, 187, 218; the
   boxes, each 217 x 27, at 489 and 88, 119, 151, 182, 213; Calculate (the default button) 276, 276, 97, 28; the result
   box (read-only) 489, 278, 217, 27. No access keys. An empty box shows the grey hint "Enter value" in italics. The
   layout never moves: the value being calculated has no box, the others keep the worksheet's own order:
   1. Mortgage: Purchase price, Down payment, Term (years), Interest rate (%), Monthly payment.
   2. Vehicle lease: Lease value, Lease period, Payments per year, Residual value, Interest rate (%), Periodic payment.
   3. Fuel economy (mpg): Distance (miles), Fuel used (gallons), Fuel economy (mpg).
   4. Fuel economy (L/100 km): Distance (kilometers), Fuel used (liters), Fuel economy (L/100 km).
   (Windows 7 ends every label with five spaces, which show nothing; Chrono Calc leaves them out.)
4. Colours and fonts: labels Segoe UI 13 px, #1E395B; boxes Segoe UI 15 px (digits 11 px high), edge #7A7A7A, white,
   text #000000; the result box #F0F0F0 with the edge #7A7A7A, #0078D7 with the keyboard focus; Calculate face #E1E1E1,
   edge #0078D7. Panel white, frame #889AAD, no line inside.
   Chrono Calc: the hint's grey (#6D6D6D) and the balloon (white, edge #767676, black text) are our own, not measured.

## B. Behaviour

1. The list, sorted, and the value chosen the first time:
   1. Mortgage: Down payment, Monthly payment, Purchase price, Term (years); first Monthly payment.
   2. Vehicle lease: Lease period, Lease value, Periodic payment, Residual value; first Periodic payment.
   3. Fuel economy (mpg): Distance (miles), Fuel economy (mpg), Fuel used (gallons); first Fuel economy (mpg).
   4. Fuel economy (L/100 km): Distance (kilometers), Fuel economy (L/100 km), Fuel used (liters); first Fuel economy
      (L/100 km).
   Choosing another value keeps every value: the old result becomes a box and the newly chosen value's number moves into
   the result box. Mortgage 250000, 50000, 30, 4,5 → 1013,370619651761, then Term (years): the boxes 250000, 50000, 4,5,
   1013,370619651761 and the result 30; choosing Monthly payment again gives the first state back.
2. The result comes only with Calculate, or Enter in a box or in the list; after it the keyboard focus is on the result
   box. Changing a box keeps the result. Escape in a box does nothing. After choosing a worksheet from the menu the
   keyboard focus is on the list.
3. The boxes start empty, hold at most 25 characters, and take digits, the decimal symbol, '-', '+' and 'e' as typed and
   pasted; every other character is left out ('12a3' → '123', '1.5' → '15', '1 234' → '1234', '1.234,5' pasted →
   '1234,5'). Nothing is grouped. ',' alone counts as 0.
4. No message box: a balloon (part D). Errors: an empty box ("This value cannot be blank."), a box that is not a valid
   number, also any negative number ("This value is not valid."), and no answer ("Unable to calculate"): interest rate 0
   in every Mortgage and Vehicle lease case, term 0, payments per year 0, a monthly payment too small to ever repay
   (Term), Lease period with the residual equal to the lease value, fuel used 0 (mpg), distance 0 (L/100 km). Calculated
   all the same: a down payment above the price (a negative payment), a residual above the lease value.
   Chrono Calc: an interest rate of 0 is calculated (the payment is the loan divided by the number of payments), because
   refusing it leaves an interest-free loan or lease without an answer (D-027).
5. A result has up to 16 digits, the decimal symbol, no grouping, '-' for a negative, no trailing zeros ('30', '125',
   '0,21'); part E line 2 for exponents.
6. Kept per worksheet: the chosen value, every box and the result, over a view change, Date calculation, Basic and back,
   and another worksheet and back. Restart: not measured (Chrono Calc remembers only the panel shown, as Windows 7's
   registry value).
7. Registry HKCU\Software\Microsoft\Calc: Templates is 1 Mortgage, 2 Vehicle lease, 3 Fuel economy (mpg), 4 Fuel economy
   (L/100 km), 0 after Basic; UnitConv and DateTime stay 0; no value or item is stored.

## C. Results

Inputs as typed, in the order of the boxes; "UC" is "Unable to calculate" (the result box emptied), "NV" is "This value
is not valid." on the box named.

### C1. Mortgage

| # | Calculate | Purchase price | Down payment | Term (years) | Interest rate (%) | Monthly payment | Result |
|---|---|---|---|---|---|---|---|
| 1 | Monthly payment | 250000 | 50000 | 30 | 4,5 | | 1013,370619651761 |
| 2 | Monthly payment | 250000 | 50000 | 30 | 0 | | UC (Chrono Calc: 555,5555555555556) |
| 3 | Monthly payment | 250000 | 50000 | 30 | 0,001 | | 555,6391245370031 |
| 4 | Monthly payment | 250000 | 50000 | 1 | 4,5 | | 17075,70431727441 |
| 5 | Monthly payment | 250000 | 50000 | 0,5 | 4,5 | | 33772,19795195795 |
| 6 | Monthly payment | 250000 | 50000 | 30 | 100 | | 16666,66666667177 |
| 7 | Monthly payment | 1000000000 | 0 | 30 | 4,5 | | 5066853,098258807 |
| 8 | Monthly payment | 100 | 0 | 1 | 12 | | 8,884878867834171 |
| 9 | Monthly payment | 250000 | 300000 | 30 | 4,5 | | -253,3426549129403 |
| 10 | Monthly payment | 250000 | 50000 | 0 | 4,5 | | UC |
| 11 | Monthly payment | 250000 | 50000 | 30 | -5 | | NV (Interest rate) |
| 12 | Monthly payment | 250000 | 50000 | 1000 | 10000 | | 1666666,666666667 |
| 13 | Monthly payment | 250000 | 50000 | 1000 | 1e4 | | 1,666666666666667e+6 |
| 14 | Monthly payment | 250000 | 50000 | 1000 | 4,5 | | 750 |
| 15 | Down payment | 250000 | | 30 | 4,5 | 1013,370619651761 | 50000,00000000008 |
| 16 | Down payment | 250000 | | 30 | 4,5 | 1000 | 52638,84099113829 |
| 17 | Down payment | 250000 | | 30 | 4,5 | 2000 | -144722,3180177234 |
| 18 | Down payment | 250000 | | 10 | 3 | 1500 | 94657,37037495967 |
| 19 | Down payment | 300000 | | 30 | 0,001 | 500 | 120027,0722776642 |
| 20 | Down payment | 250000 | | 30 | 0 | 1000 | UC (Chrono Calc: -110000) |
| 21 | Down payment | 250000 | | 0,5 | 4,5 | 33772,19795195795 | 50000,00000000002 |
| 22 | Down payment | 100 | | 1 | 12 | 1 | 88,74492252651537 |
| 23 | Purchase price | | 50000 | 30 | 4,5 | 1013,370619651761 | 249999,9999999999 |
| 24 | Purchase price | | 0 | 30 | 4,5 | 1000 | 197361,1590088617 |
| 25 | Purchase price | | 50000 | 15 | 6 | 2000 | 287007,0293352141 |
| 26 | Purchase price | | 0 | 30 | 0 | 1000 | UC (Chrono Calc: 360000) |
| 27 | Purchase price | | 0 | 1 | 12 | 8,884878867834171 | 100 |
| 28 | Purchase price | | 0 | 0,5 | 4,5 | 100 | 592,2030904962315 |
| 29 | Purchase price | | 0 | 30 | 100 | 1000 | 11999,99999999633 |
| 30 | Purchase price | | -1000 | 30 | 4,5 | 1000 | NV (Down payment) |
| 31 | Term (years) | 250000 | 50000 | | 4,5 | 1013,370619651761 | 30,00000000000002 |
| 32 | Term (years) | 250000 | 50000 | | 4,5 | 1500 | 15,43213379469031 |
| 33 | Term (years) | 250000 | 50000 | | 4,5 | 750 | UC (750 is the monthly interest) |
| 34 | Term (years) | 250000 | 50000 | | 4,5 | 500 | UC |
| 35 | Term (years) | 250000 | 50000 | | 0 | 1000 | UC (Chrono Calc: 16,66666666666667) |
| 36 | Term (years) | 100 | 0 | | 12 | 8,884878867834171 | 1 |
| 37 | Term (years) | 250000 | 50000 | | 0,001 | 1000 | 16,66806265491808 |
| 38 | Term (years) | 250000 | 0 | | 6 | 1250 | UC (1250 is the monthly interest) |
| 39 | Term (years) | 100000 | 150000 | | 5 | 500 | -6,980636606914711 |
| 40 | Term (years) | 250000 | 50000 | | 4,5 | 0 | UC |

The formula (every result above matches it to all 16 digits): i = rate / 1200, n = term * 12, loan = price - down
payment, payment = loan * i / (1 - (1 + i)^-n), paid at the end of each month; Term from (1 + i)^-n = 1 - loan * i /
payment.

### C2. Vehicle lease

| # | Calculate | Lease value | Lease period | Payments per year | Residual value | Interest rate (%) | Periodic payment | Result |
|---|---|---|---|---|---|---|---|---|
| 1 | Periodic payment | 30000 | 3 | 12 | 10000 | 5 | | 641,084608759976 |
| 2 | Periodic payment | 30000 | 3 | 12 | 10000 | 0 | | UC (Chrono Calc: 555,5555555555556) |
| 3 | Periodic payment | 30000 | 3 | 12 | 0 | 5 | | 899,1269131399641 |
| 4 | Periodic payment | 30000 | 3 | 12 | 30000 | 5 | | 125 |
| 5 | Periodic payment | 30000 | 3 | 1 | 10000 | 5 | | 7844,171292624901 |
| 6 | Periodic payment | 30000 | 3 | 4 | 10000 | 5 | | 1930,166246903139 |
| 7 | Periodic payment | 30000 | 3 | 52 | 10000 | 5 | | 147,7377037974495 |
| 8 | Periodic payment | 30000 | 3 | 7 | 10000 | 5 | | 1100,414125707024 |
| 9 | Periodic payment | 30000 | 3 | 0 | 10000 | 5 | | UC |
| 10 | Periodic payment | 30000 | 3 | 12 | 40000 | 5 | | -133,042304379988 |
| 11 | Periodic payment | 30000 | 3 | 7,5 | 10000 | 5 | | 1026,842421142456 |
| 12 | Periodic payment | 30000 | 0,1 | 12 | 10000 | 5 | | 16784,73280974062 |
| 13 | Lease value | | 3 | 12 | 10000 | 5 | 641,084608759976 | 30000 |
| 14 | Lease value | | 3 | 12 | 0 | 5 | 500 | 16682,8506418704 |
| 15 | Lease value | | 3 | 12 | 10000 | 0 | 500 | UC (Chrono Calc: 28000) |
| 16 | Lease value | | 3 | 1 | 10000 | 5 | 7844,171292624901 | 30000 |
| 17 | Lease value | | 5 | 12 | 5000 | 3,5 | 300 | 20689,35069908491 |
| 18 | Lease value | | 3 | 7 | 10000 | 5 | 1100,414125707024 | 30000 |
| 19 | Lease value | | 0,5 | 12 | 0 | 5 | 1000 | 5913,463183017656 |
| 20 | Lease value | | 3 | 12 | 10000 | 5 | 0 | 8609,7624465108 |
| 21 | Residual value | 30000 | 3 | 12 | | 5 | 641,084608759976 | 10000 |
| 22 | Residual value | 30000 | 3 | 12 | | 5 | 0 | 34844,16694000405 |
| 23 | Residual value | 30000 | 3 | 12 | | 5 | 1000 | -3909,168580028343 |
| 24 | Residual value | 30000 | 3 | 12 | | 0 | 500 | UC (Chrono Calc: 12000) |
| 25 | Residual value | 30000 | 3 | 1 | | 5 | 7844,171292624901 | 10000 |
| 26 | Residual value | 20000 | 4 | 4 | | 6 | 1200 | 3860,867140748894 |
| 27 | Residual value | 30000 | 3 | 12 | | 5 | 125 | 30000 |
| 28 | Residual value | 30000 | 3 | 52 | | 5 | 147,7377037974495 | 9999,999999999994 |
| 29 | Lease period | 30000 | | 12 | 10000 | 5 | 641,084608759976 | 3 |
| 30 | Lease period | 30000 | | 12 | 10000 | 5 | 800 | 2,333054106858931 |
| 31 | Lease period | 30000 | | 12 | 10000 | 5 | 100 | UC |
| 32 | Lease period | 30000 | | 12 | 10000 | 0 | 500 | UC (Chrono Calc: 3,333333333333333) |
| 33 | Lease period | 30000 | | 1 | 10000 | 5 | 7844,171292624901 | 3 |
| 34 | Lease period | 30000 | | 12 | 0 | 5 | 1000 | 2,676187804789212 |
| 35 | Lease period | 30000 | | 12 | 30000 | 5 | 125 | UC |
| 36 | Lease period | 30000 | | 4 | 10000 | 5 | 1500 | 4,038445812231709 |
| 37 | Lease period | 30000 | | 12 | 40000 | 5 | 100 | 19,65742462427103 |

The formula (every result above matches it to all 16 digits): i = rate / (100 * payments per year), n = period *
payments per year, payment = (value - residual * (1 + i)^-n) * i / (1 - (1 + i)^-n), paid at the end of each period.

### C3. Fuel economy

| # | Worksheet | Calculate | Distance | Fuel used | Fuel economy | Result |
|---|---|---|---|---|---|---|
| 1 | mpg | Fuel economy | 100 | 3 | | 33,33333333333333 |
| 2 | mpg | Fuel economy | 300 | 10 | | 30 |
| 3 | mpg | Fuel economy | 1 | 3 | | 0,3333333333333333 |
| 4 | mpg | Fuel economy | 0 | 5 | | 0 |
| 5 | mpg | Fuel economy | 100 | 0 | | UC |
| 6 | mpg | Fuel economy | 9999999999999999 | 1 | | 9999999999999999 |
| 7 | mpg | Fuel economy | 2 | 3 | | 0,6666666666666667 |
| 8 | mpg | Distance | | 10 | 30 | 300 |
| 9 | mpg | Distance | | 3 | 33,33333333333333 | 99,99999999999999 |
| 10 | mpg | Distance | | 0 | 30 | 0 |
| 11 | mpg | Distance | | 10 | 0 | 0 |
| 12 | mpg | Distance | | 1,5 | 2,5 | 3,75 |
| 13 | mpg | Distance | | 7 | 3 | 21 |
| 14 | mpg | Fuel used | 300 | | 30 | 10 |
| 15 | mpg | Fuel used | 100 | | 3 | 33,33333333333333 |
| 16 | mpg | Fuel used | 0 | | 30 | 0 |
| 17 | mpg | Fuel used | 100 | | 0 | UC |
| 18 | mpg | Fuel used | 1 | | 7 | 0,1428571428571429 |
| 19 | mpg | Fuel used | 2,5 | | 1,5 | 1,666666666666667 |
| 20 | L/100 km | Fuel economy | 100 | 3 | | 3 |
| 21 | L/100 km | Fuel economy | 300 | 10 | | 3,333333333333333 |
| 22 | L/100 km | Fuel economy | 3 | 1 | | 33,33333333333333 |
| 23 | L/100 km | Fuel economy | 0 | 5 | | UC |
| 24 | L/100 km | Fuel economy | 100 | 0 | | 0 |
| 25 | L/100 km | Fuel economy | 7 | 1 | | 14,28571428571429 |
| 26 | L/100 km | Fuel economy | 9999999999999999 | 1 | | 0,00000000000001 |
| 27 | L/100 km | Distance | | 10 | 5 | 200 |
| 28 | L/100 km | Distance | | 3 | 3,333333333333333 | 90,00000000000001 |
| 29 | L/100 km | Distance | | 0 | 5 | 0 |
| 30 | L/100 km | Distance | | 10 | 0 | UC |
| 31 | L/100 km | Distance | | 1 | 7 | 14,28571428571429 |
| 32 | L/100 km | Distance | | 2,5 | 1,5 | 166,6666666666667 |
| 33 | L/100 km | Fuel used | 200 | | 5 | 10 |
| 34 | L/100 km | Fuel used | 100 | | 3 | 3 |
| 35 | L/100 km | Fuel used | 0 | | 5 | 0 |
| 36 | L/100 km | Fuel used | 100 | | 0 | 0 |
| 37 | L/100 km | Fuel used | 7 | | 3 | 0,21 |
| 38 | L/100 km | Fuel used | 1 | | 1 | 0,01 |

mpg = distance / fuel used; L/100 km = fuel used / distance * 100.

## D. Messages (follow-up)

1. The message is a balloon below the box it is about (for the 4th box at 486, 203; for the result box at 486, 299,
   153 x 52), and the keyboard focus goes to that box: "This value cannot be blank." and "This value is not valid." at
   the box, "Unable to calculate" at the result box.
2. All boxes are checked for a text that is not a valid number first: the first such box wins over any empty box; only
   then the first empty box is reported (2nd and 4th empty: the 2nd; 1st empty and 3rd not valid: the 3rd; 2nd not valid
   and 4th empty: the 2nd).
3. "This value is not valid." keeps the result box as it was; "This value cannot be blank." and "Unable to calculate"
   empty it. The boxes keep their text, also a text that is not valid.
4. The balloon goes after 8.7 seconds (8686 and 8736 ms measured), with Right in its box, and with a click into another
   box; typing into its box keeps it ('-5' became '-51' with the balloon still up).
   Chrono Calc: Left, Home and End in the box remove it as well (they move the cursor as Right does; not measured).

## E. Numbers and how a result is written (follow-up)

1. Fuel economy (mpg), Distance as given, Fuel used 1 (after 7 / 1 was calculated, so a kept result shows 7):

| # | Distance | Result |
|---|---|---|
| 1 | 1e5 | 1,e+5 |
| 2 | 1,5e3 | 1,5e+3 |
| 3 | 1e-5 | 1,e-5 |
| 4 | 1e400 | 1,e+400 |
| 5 | 1e+5 | 1,e+5 |
| 6 | 12345678901234567 | NV |
| 7 | 1234567890123456 | 1234567890123456 |
| 8 | 0,12345678901234567 | NV |
| 9 | 00000000000000001 | 1 |
| 10 | +5 | 5 |
| 11 | --5 | NV |
| 12 | 1,5,5 | NV |
| 13 | 5- | NV |
| 14 | e5 | 1,e+5 |
| 15 | 1e | 1,e+0 |
| 16 | -0 | NV |
| 17 | 0,0000000000000000001 | NV |
| 18 | 100000 | 100000 |

   A number has at most 16 digits: zeros before the first other digit of the whole part do not count, every digit after
   the decimal symbol does. Without digits before the exponent the number is 1 ("e5"), an exponent without digits is 0.
   A number typed with an exponent makes the result written with one ("1,e+5"; 100000 / 1 is "100000").
   Chrono Calc: an exponent has at most four digits, as the calculator's own exponent input (Windows 7 took 1e400; a
   longer one was not measured).
2. Huge and tiny results (mpg): 9999999999999999 / 0,0001 is "9,999999999999999e+19"; 1 / 9999999999999999 is
   "0,0000000000000001"; 1e300 / 1e-300 is "1,e+600"; 1e300 / 1e-5 is "1,e+305"; 1 / 3e-20 is "3,333333333333333e+19".
   Chrono Calc: a result is worked at 32, then 64 (then 128) digits and shown when two of them give the same 16 digits;
   otherwise "Unable to calculate"; 1 - (1 + i)^-n is worked so that a rate near 0 keeps its digits (not seen in
   Windows 7's results). A result below 1 written with an exponent keeps 16 digits ("2 / 3e20"
   is 6,666666666666667e-21; not measured in Windows 7, whose engine writes fewer digits there). (1 + i)^-n too small
   for the number library (a term of 10 million years) counts as nothing beside the other terms (750 for C1 line 14
   with any longer term); where it would be the whole answer, "Unable to calculate".

## F. Keys

1. Tab order (Mortgage): the boxes in order, Calculate, the result box, the calculator, the list, the first box again.
2. The list: Up and Down change the value at once (with the move of part B line 1), F4 opens it, Escape closes it
   unchanged, Enter is Calculate; digit keys and typed characters do nothing (the display stays 0, the value stays;
   unlike the Unit conversion lists).
3. The result box takes the keyboard focus with a click; select-all and Ctrl+C copy its text.
4. Digits typed into a box go only into the box; the display stays 0.

## G. Odd values (follow-up)

1. Mortgage Term with Purchase price 100000, Down payment 150000, rate 5, Monthly payment 500 (a negative loan):
   "-6,980636606914711" (C1 line 39).
2. Mortgage Term with Monthly payment 0: "Unable to calculate" (C1 line 40).
3. Lease period 30000 / 12 / 40000 / 5 / 100: "19,65742462427103" (C2 line 37).
4. Payments per year and Lease period take fractions: C2 lines 11 and 12.
5. Interest rate -5: "This value is not valid." on the rate box, the result kept (C1 line 11).
6. Rate 10000 and term 1000: "1666666,666666667"; the rate typed as 1e4: "1,666666666666667e+6" (C1 lines 12 and 13).
