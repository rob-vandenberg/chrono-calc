# REQUIREMENTS.md

What Chrono Calc must do, as the owner decided it. The basic rule is in `AGENTS.md` §0 (behave like the Windows 7 Calculator); the design decisions are in `DECISIONS.md`. This file holds the requirements a reviewer checks a change against.

## Brackets in the Standard view (D-029, the owner, 2026-11-21)

An exception to "behave like the Windows 7 Calculator": Windows 7's Standard view has no brackets.

1. In the Standard view, `(` and `)` typed on the keyboard open and close a bracket.
2. There are no bracket buttons; the Standard view looks exactly as before.
3. Edit > Paste types the text as keys, so pasted brackets work the same way.
4. The Standard view still calculates left to right, inside and outside brackets: `(3+4)*(8+2)=` gives 70, `2+3*(1+1)=` gives 10.
5. The line above the number shows the brackets as typed, tight against what they hold, as the Scientific view writes them: `(8 * 4 + 7) *`.
6. `=` closes any brackets still open: `(3+4*2=` gives 14.
7. A `)` without an open bracket does nothing.
8. The history entry keeps the brackets (`(3 + 4) * (8 + 2)`), and History > Edit calculates such an entry again.
9. A value right before `(` is multiplied, as the engine does in the Scientific view: `3(4)=` gives 12.
