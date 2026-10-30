# Windows 7 Calculator: Programmer view reference

Measured on 2026-10-30 on the owner's Windows computer, in the original Windows 7 Calculator (calc1.exe), through UI
Automation, posted keys and button clicks (task M3). Physical pixels at 100%; the decimal separator there is a comma.

## A. Window layout

Outer 526 x 485; client 508 x 413 (origin 9,63): 82 px taller than Standard and Scientific. Rectangles: left, top,
width, height. The display is as in Scientific, but its expression line stays empty in Programmer.

| Id | What | Rectangle |
|---|---|---|
| 1077 | bit display frame | 13, 82, 482, 77 |
| 763 to 700 | bits 63 to 0, each 9 x 15 | rows at top 88 (63-32) and 121 (31-0); groups of four at left 22, 82, 142, 202, 262, 322, 382, 442; bits at +0, +10, +19, +29 |
| 1076 to 1071 | bit positions 63, 47, 32 / 31, 15, 0 (18 x 17) | 22,103; 262,103; 461,103 (right-aligned); 22,136; 262,136; 461,136 (right-aligned) |
| 1079 | base frame | 13, 165, 92, 115: Hex 22,175; Dec 22,200; Oct 22,225; Bin 22,250 |
| 1080 | word size frame | 13, 284, 92, 115: Qword 22,293; Dword 22,318; Word 22,343; Byte 22,368 |
| 402 | bracket counter | 111, 165, 43, 33 |

Buttons 43 x 33; rows 165, 205, 245, 284, 325, 364; columns 111, 160, 208, 257, 306, 355, 403, 452.

| Row | Buttons |
|---|---|
| 165 | (counter) Mod, A, MC, MR, MS, M+, M- |
| 205 | (, ), B, backspace, CE, C, ±, √ |
| 245 | RoL, RoR, C, 7, 8, 9, /, % |
| 284 | Or, Xor, D, 4, 5, 6, *, 1/x |
| 325 | Lsh, Rsh, E, 1, 2, 3, -, = (43 x 74) |
| 364 | Not, And, F, 0 (92 x 33), decimal point, + |

Always greyed: the decimal point, √, % and 1/x. Dec greys A-F; Oct greys A-F, 8, 9; Bin greys A-F and 2-9. The word
size greys nothing: bits beyond it are hidden. A-F are flat (#D9E4F1) with digit-sized labels; the operators have the
function face. Bits: Consolas 15 px, #1E395B; bit positions #666666; frames #8696AA on #D9E4F1.

## B. Behaviour

1. A view change clears the number and keeps the memory; the base and word size are kept (the angle unit of
   Scientific is not).
2. A click on a bit turns it over. The decimal point is ignored (2,5 gives 25); 7/2= gives 3.
3. Dec is signed two's complement: 5 ± is -5, in Hex FFFFFFFFFFFFFFFB, in Byte FB. Results wrap (Byte 127+1= is -128,
   Qword 9223372036854775807+1= is -9223372036854775808); a digit that would leave the range is refused (Byte 255
   types as 25; Qword 18446744073709551615 types as 1844674407370955161). A smaller word size cuts the number to its
   low bits (300 in Byte is 44) and they do not come back.
4. Bin Byte takes at most 8 digits; Hex Qword 16. Numbers are shown without grouping.

## C. Keys (Programmer)

F5 Hex, F6 Dec, F7 Oct, F8 Bin; F12 Qword, F2 Dword, F3 Word, F4 Byte; the keys A-F; & And, | Or, ^ Xor, ~ Not
(at once), < Lsh, > Rsh, % Mod, ( and ); J RoL, K RoR; memory, F9, Delete, Backspace, Escape as in Standard. R (1/x),
@ (√), ; ! # and the other letters do nothing.

## D. Results

In `tests/test_reference_programmer.cpp`: precedence (12+3*4= is 24), integer division, Mod, And, Or, Xor, Not, Lsh,
Rsh, 8-bit and 64-bit rotations, Hex/Oct/Bin arithmetic, conversions between bases, negative numbers, wrapping, 1/0=
("Cannot divide by zero", expression line empty), 0 Mod 0 and 5 Mod 0 ("Result is undefined"), 1 Lsh 64 ("Result not
defined"), brackets, and % as Mod.

## E. Menus

The dot is on View > Programmer; View > History is greyed in Programmer.

## F. Memory across views and letter keys

1. The memory keeps its value over a view change; Programmer stores in two's complement of the word size: -1 stored in
   Programmer (Qword) is 1,844674407370955e+19 in Standard, in Byte 255; FF stored in Hex is 255 in Standard.
2. A fraction stored in Standard (2,5) is recalled truncated in Programmer (2) and whole again in Standard (2,5).
3. Windows 7 fault, not copied: -1,5 stored in Standard is recalled in Programmer as 1844674407370955161 (Hex
   1999999999999999), while -2,5 gives -2 and -1 gives -1; ours gives -1, the same truncation as every other number.
4. The hexadecimal digits come from the letter keys, with or without Shift and with Caps Lock; in Scientific the letters
   work with Caps Lock on, not with Shift.
5. On a Russian keyboard layout (measured 2026-10-30, Russian active only for the calculator) every letter shortcut
   keeps working, because Windows 7 reads the key, whose virtual key stays at its Latin place (the S key, scan code
   0x1F, is VK S and types "ы"): 30 then S gives 0,5 and sind(30); Ctrl+M stores 7 (M shows), Ctrl+R recalls 7, Ctrl+L
   clears the memory; A to F (ф и с в у а) in Hex give ABCDEF. C alone is not Clear in Standard, as on a Latin layout.
6. Greek and Hebrew (measured 2026-10-30, the same way): a key's shortcut on its Latin place wins; only where the view
   has no shortcut for that letter does the typed character count. Greek Q types ";": Scientific 3 then Q gives 9 and
   sqr(3) (x², not Int); Standard 8 then Q does nothing. Hebrew Q types "/": Standard 8 Q 2 Enter gives 4 (8 / shows);
   Scientific 3 then Q gives 9 and sqr(3), no divide; S (ד) is sin (30 gives 0,5 and sind(30)); A (ש) in Hex gives A.
