#include <QObject>
#include <QLatin1Char>
#include <QString>
#include <QStringList>
#include <QTest>

#include "core/calculator.h"
#include "core/commands.h"
#include "core/enginetexts.h"
#include "core/keys.h"

#include <cstddef>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// tests/test_reference_history.cpp 0.5.12

// --- Version history ----------------------------------------------------
// v0.5.12: Initial: the Windows 7 History, Digit grouping, Copy and Paste results of tests/reference/windows7-history.md

// Each line of tests/reference/windows7-history.md, measured in the Windows 7 Calculator (regional settings with a
// decimal comma and a thousands point): what was done and what Windows 7 showed. A step is a command id, a command with
// its text ("edit.paste=2+3=", where _ stands for a space, | for a line end and ~ for a carriage return), or characters
// typed as Windows 7 reads them in the current view.
namespace {

constexpr chrono_calc::Separators commaSeparators{.decimal = L',', .thousands = L'.'};

// What a line expects; nullptr where the line does not look at it
struct Line
{
    const char* name;
    const char* steps;
    const char* display;
    const char* expression = nullptr;
    // The history's entries, joined with " | " ("" for none)
    const char* history = nullptr;
    // The text the last copy command gave, line ends as |
    const char* copied = nullptr;
};

std::vector<Line> lines()
{
    return {
        // Part B: what History records
        {"B1 2+3=", "2+3=", "5", "", "2 + 3"},
        {"B2 2+3*4=", "2+3*4=", "20", "", "2 + 3 * 4"},
        {"B3 5=", "5=", "5", "", "5"},
        {"B4 5+=", "5+=", "10", "", "5 + 5"},
        {"B5 2+3==", "2+3==", "8", "", "2 + 3 | 5 + 3"},
        {"B6 9 sqrt", "9 sqrt", "3", "sqrt(9)", ""},
        {"B6 9 sqrt, a digit", "9 sqrt 5", "5", "", "sqrt(9)"},
        {"B6 9 sqrt, CE", "9 sqrt clear_entry", "0", "", "sqrt(9)"},
        {"B6 4 sqrt sqrt, CE", "4 sqrt sqrt clear_entry", "0", "", "sqrt(sqrt(4))"},
        {"B6 9 sqrt, C", "9 sqrt clear", "0", "", ""},
        {"B7 9 sqrt +1=", "9 sqrt +1=", "4", "", "sqrt(9) + 1"},
        {"B8 100+5%=", "100+5%=", "105", "", "100 + 5"},
        {"B9 1/0=", "1/0=", "Cannot divide by zero", "1 /", ""},
        {"B10 2 F9 +3=", "2 negate +3=", "1", "", "-2 + 3"},
        {"B11 MS 7, 2+MR=", "7 memory.store clear 2+ memory.recall =", "9", "", "2 + 7"},
        {"B12 1/3=", "1/3=", "0,3333333333333333", "", "1 / 3"},
        {"B13 Scientific 2+3*4=", "view.scientific 2+3*4=", "14", "", "2 + 3 * 4"},
        {"B14 Scientific 30 sin +1=", "view.scientific 30s+1=", "1,5", "", "sind(30) + 1"},
        {"B15 Scientific (2+3)*4=", "view.scientific (2+3)*4=", "20", "", "(2 + 3) * 4"},
        {"B16 Scientific 2 x^y 10=", "view.scientific 2y10=", "1024", "", "2 ^ 10"},
        {"B17 Scientific 5 n! =", "view.scientific 5!=", "120", "", "fact(5)"},
        {"B18 2+3 Escape", "2+3 clear", "0", "", ""},
        {"B note 9 sqrt 5", "9 sqrt 5", "5", ""},
        {"B note 9 sqrt CE", "9 sqrt clear_entry", "0", ""},
        {"B note 2+9 sqrt 5", "2+9 sqrt 5", "5", "2 +"},
        {"B2 one history per view", "2+3= view.scientific 2*4= view.standard", "0", "", "2 + 3"},
        {"B2 kept over Programmer", "view.scientific 2*4= view.programmer view.scientific", "0", "", "2 * 4"},
        {"B2 C and CE keep it", "2+3= clear clear_entry", "0", "", "2 + 3"},
        // Part C: using History
        {"C1 a click shows the result", "2+3= 2*4= history.select.0", "5", "", "2 + 3 | 2 * 4"},
        {"C1 the expression line stays", "1/3= 2+ history.select.0", "0,3333333333333333", "2 +", "1 / 3"},
        {"C1 then =", "1/3= 2+ history.select.0 =", "2,333333333333333", "", "1 / 3 | 2 + 0,3333333333333333"},
        {"C2 the shown result, not the exact one", "1/3= 2*4= history.select.0 *3=", "0,9999999999999999", "",
         "1 / 3 | 2 * 4 | 0,3333333333333333 * 3"},
        {"C2 a chosen result as a plain number", "2+3= history.select.0 +1=", "6", "", "2 + 3 | 5 + 1"},
        {"C3 Up with History shown", "view.history 2+3= 2*4= 1/3= history.select_previous", "8"},
        {"C3 Up, then Down", "view.history 2+3= 2*4= 1/3= history.select_previous history.select_next", "0,3333333333333333"},
        {"C3 Up at the first entry", "view.history 2+3= history.select_previous history.select_previous", "5"},
        {"C3 Up with History hidden", "2+3= 2*4= history.select_previous", "8"},
        {"C3 Up with History hidden does nothing", "2+3= 2*4= 7 history.select_previous", "7"},
        {"C5 a click during an error", "2+3= 1/0= history.select.0", "5", "", "2 + 3"},
        {"C5 Up during an error", "view.history 2+3= 2*4= 1/0= history.select_previous", "5"},
        {"C5 F2 during an error", "view.history 2+3= 1/0= history.edit", "Cannot divide by zero", "1 /"},
        {"C5 Clear during an error", "2+3= 1/0= history.clear", "Cannot divide by zero", "1 /", ""},
        {"C6 Copy history", "2+3= 2*4= history.copy", "8", "", "2 + 3 | 2 * 4", "2 + 3|2 * 4|"},
        {"C7 Clear keeps the number", "6*7= history.clear", "42", "", ""},
        // Part D: History Edit (F2), with History shown; not accepted leaves the entry and the display as they were
        {"D1 2+3*4", "view.history 2+3= history.edit history.recalculate=2+3*4", "20", "", "2 + 3 * 4"},
        {"D2 (2+3)*4", "view.history 2+3= history.edit history.recalculate=(2+3)*4", "20", "", "(2 + 3) * 4"},
        {"D3 sqrt(9)+1", "view.history 2+3= history.edit history.recalculate=sqrt(9)+1", "4", "", "sqrt(9) + 1"},
        {"D4 sqrt 9", "view.history 2+3= history.edit history.recalculate=sqrt_9", "5", "", "2 + 3"},
        {"D5 1,5+2", "view.history 2+3= history.edit history.recalculate=1,5+2", "3,5", "", "1,5 + 2"},
        {"D6 1.5+2", "view.history 2+3= history.edit history.recalculate=1.5+2", "17", "", "15 + 2"},
        {"D7 -2+3", "view.history 2+3= history.edit history.recalculate=-2+3", "1", "", "-2 + 3"},
        {"D8 2+-3", "view.history 2+3= history.edit history.recalculate=2+-3", "-1", "", "2 + -3"},
        {"D9 7", "view.history 2+3= history.edit history.recalculate=7", "7", "", "7"},
        {"D10 empty", "view.history 2+3= history.edit history.recalculate=", "5", "", "2 + 3"},
        {"D11 spaces", "view.history 2+3= history.edit history.recalculate=__2__+__3__", "5", "", "2 + 3"},
        {"D12 1/0", "view.history 2+3= history.edit history.recalculate=1/0", "5", "", "2 + 3"},
        {"D13 reciproc(4)", "view.history 2+3= history.edit history.recalculate=reciproc(4)", "0,25", "", "reciproc(4)"},
        {"D14 negate(2)+1", "view.history 2+3= history.edit history.recalculate=negate(2)+1", "-1", "", "negate(2) + 1"},
        {"D15 2^10", "view.history 2+3= history.edit history.recalculate=2^10", "1024", "", "2 ^ 10"},
        {"D16 fact(5)", "view.history 2+3= history.edit history.recalculate=fact(5)", "120", "", "fact(5)"},
        {"D17 50%", "view.history 2+3= history.edit history.recalculate=50%", "5", "", "2 + 3"},
        {"D17 100+5%", "view.history 2+3= history.edit history.recalculate=100+5%", "5", "", "2 + 3"},
        {"D18 20 digits", "view.history 2+3= history.edit history.recalculate=12345678901234567890+1", "5", "", "2 + 3"},
        {"D19 Scientific sind(30)+1", "view.history view.scientific 2+3= history.edit history.recalculate=sind(30)+1", "1,5", "", "sind(30) + 1"},
        {"D20 Scientific 2 ^ 10", "view.history view.scientific 2+3= history.edit history.recalculate=2_^_10", "1024", "", "2 ^ 10"},
        {"D21 Scientific fact(5)", "view.history view.scientific 2+3= history.edit history.recalculate=fact(5)", "120", "", "fact(5)"},
        {"D22 Scientific 10 Mod 3", "view.history view.scientific 2+3= history.edit history.recalculate=10_Mod_3", "1", "", "10 Mod 3"},
        {"D23 Scientific 1,e+5", "view.history view.scientific 2+3= history.edit history.recalculate=1,e+5", "100000", "", "100000"},
        {"D24 Scientific 2 yroot 8", "view.history view.scientific 2+3= history.edit history.recalculate=2_yroot_8",
         "1,0905077326652576592070106557607", "", "2 yroot 8"},
        {"D25 Scientific sqr(3)", "view.history view.scientific 2+3= history.edit history.recalculate=sqr(3)", "9", "", "sqr(3)"},
        {"D25 Scientific cube(2)", "view.history view.scientific 2+3= history.edit history.recalculate=cube(2)", "8", "", "cube(2)"},
        {"D25 Scientific powten(2)", "view.history view.scientific 2+3= history.edit history.recalculate=powten(2)", "100", "", "powten(2)"},
        {"D26 Scientific sin(30)", "view.history view.scientific 2+3= history.edit history.recalculate=sin(30)", "5", "", "2 + 3"},
        {"D26 Scientific (2+3", "view.history view.scientific 2+3= history.edit history.recalculate=(2+3", "5", "", "2 + 3"},
        {"D26 Scientific 2+3)", "view.history view.scientific 2+3= history.edit history.recalculate=2+3)", "5", "", "2 + 3"},
        {"D26 Scientific 5!", "view.history view.scientific 2+3= history.edit history.recalculate=5!", "5", "", "2 + 3"},
        {"D26 Scientific pi", "view.history view.scientific 2+3= history.edit history.recalculate=pi", "5", "", "2 + 3"},
        {"D3 the middle entry", "view.history 1/3= 2*4= 5-1= history.select.1 history.edit history.recalculate=6*7", "42", "",
         "1 / 3 | 6 * 7 | 5 - 1"},
        {"D2 Cancel edit", "view.history 2*4= history.edit history.cancel_edit history.recalculate=6*7", "8", "", "2 * 4"},
        {"D1 F2 with History hidden", "2+3= history.edit history.recalculate=6*7", "5", "", "2 + 3"},
        {"D1 F2 with an empty history", "view.history 7 history.edit history.recalculate=6*7", "7", "", ""},
        // Part E: Digit grouping
        {"E2 typing 1234", "view.digit_grouping 1234", "1.234"},
        {"E2 typing 1234567", "view.digit_grouping 1234567", "1.234.567"},
        {"E2 1234567,891", "view.digit_grouping 1234567,891", "1.234.567,891"},
        {"E2 -1234567", "view.digit_grouping 1234567 negate", "-1.234.567"},
        {"E2 1234567*1000=", "view.digit_grouping 1234567*1000=", "1.234.567.000"},
        {"E2 1/3=", "view.digit_grouping 1/3=", "0,3333333333333333"},
        {"E2 1,e+20", "view.digit_grouping 10000000000*10000000000=", "1,e+20"},
        {"E2 1,e+16", "view.digit_grouping 9999999999999999+1=", "1,e+16"},
        {"E2 an error", "view.digit_grouping 1/0=", "Cannot divide by zero"},
        {"E3 the expression line", "view.digit_grouping 1234567+", "1.234.567", "1234567 +"},
        {"E3 the history", "view.digit_grouping 1234567+1=", "1.234.568", "", "1234567 + 1"},
        {"E3 an operator after a result", "view.digit_grouping 1234567*1= +", "1.234.567", "1234567 +"},
        {"E4 Scientific 123456789", "view.scientific view.digit_grouping 123456789", "123.456.789"},
        {"E4 Scientific 2/3=", "view.scientific view.digit_grouping 2/3=", "0,66666666666666666666666666666667"},
        {"E4 Scientific F-E typing", "view.scientific view.digit_grouping v1234567", "1.234.567"},
        {"E4 Scientific 1234567 Exp 5", "view.scientific view.digit_grouping 1234567x5", "1.234.567,e+5"},
        {"E4 Scientific 1234567 Exp 5 =", "view.scientific view.digit_grouping 1234567x5=", "123.456.700.000"},
        {"E5 Hex FFFFFFFF", "view.programmer view.digit_grouping base.hexadecimal FFFFFFFF", "FFFF FFFF"},
        {"E5 Dec 1234567", "view.programmer view.digit_grouping 1234567", "1.234.567"},
        {"E5 Dec -1234567", "view.programmer view.digit_grouping 1234567 negate", "-1.234.567"},
        {"E5 Oct 1234567", "view.programmer view.digit_grouping base.octal 1234567", "1 234 567"},
        {"E5 Bin 11111111111", "view.programmer view.digit_grouping base.binary 11111111111", "111 1111 1111"},
        {"E5 Hex without grouping", "view.programmer base.hexadecimal FFFFFFFF", "FFFFFFFF"},
        {"E6 Statistics", "view.statistics view.digit_grouping 1234567", "1.234.567"},
        {"E1 on while a number shows", "1234 view.digit_grouping", "1.234"},
        {"E1 on while a result shows", "1234*1= view.digit_grouping", "1.234"},
        {"E1 off again", "view.digit_grouping 1234567 view.digit_grouping", "1234567"},
        {"E1 during an error", "1/0= view.digit_grouping", "Cannot divide by zero"},
        // Part F: Copy and Paste
        {"F1 Copy", "1234567,5 edit.copy", "1234567,5", nullptr, nullptr, "1234567,5"},
        {"F1 Copy with grouping", "view.digit_grouping 1234567,5 edit.copy", "1.234.567,5", nullptr, nullptr, "1.234.567,5"},
        {"F1 Copy an error", "1/0= edit.copy", "Cannot divide by zero", nullptr, nullptr, "Cannot divide by zero"},
        {"F1 Copy with 2 + pending", "2+3 edit.copy", "3", "2 +", nullptr, "3"},
        {"F1 Copy F-E", "view.scientific 5v edit.copy", "5,e+0", nullptr, nullptr, "5,e+0"},
        {"F1 Copy Hex FF", "view.programmer base.hexadecimal FF edit.copy", "FF", nullptr, nullptr, "FF"},
        {"F1 Copy Hex with grouping", "view.programmer view.digit_grouping base.hexadecimal FFFFFFFF edit.copy", "FFFF FFFF",
         nullptr, nullptr, "FFFF FFFF"},
        {"F1 Copy Bin with grouping", "view.programmer view.digit_grouping base.binary 111111111 edit.copy", "1 1111 1111",
         nullptr, nullptr, "1 1111 1111"},
        {"F2 Copy dataset", "view.statistics 5 statistics.add 3 statistics.add 4 statistics.add statistics.copy_dataset", "0",
         nullptr, nullptr, "5|3|4|"},
        {"F3-1 123", "edit.paste=123", "123", ""},
        {"F3-2 1,5", "edit.paste=1,5", "1,5", ""},
        {"F3-3 1.5", "edit.paste=1.5", "15", ""},
        {"F3-4 1.234,5", "edit.paste=1.234,5", "1234,5", ""},
        {"F3-5 1 234", "edit.paste=1_234", "1234", ""},
        {"F3-6 -5", "edit.paste=-5", "-5", ""},
        {"F3-7 space 5 space", "edit.paste=_5_", "5", ""},
        {"F3-8 5-", "edit.paste=5-", "5", "5 -"},
        {"F3-9 abc", "edit.paste=abc", "0", ""},
        {"F3-10 12a3", "edit.paste=12a3", "123", ""},
        {"F3-11 2+3=", "edit.paste=2+3=", "5", "", "2 + 3"},
        {"F3-12 2+3", "edit.paste=2+3", "3", "2 +"},
        {"F3-13 2*3=4=", "edit.paste=2*3=4=", "12", "", "2 * 3 | 4 * 3"},
        {"F3-14 20 digits", "edit.paste=12345678901234567890", "1234567890123456"},
        {"F3-15 1e5", "edit.paste=1e5", "15"},
        {"F3-16 1,2e+3", "edit.paste=1,2e+3", "1,23", ""},
        {"F3-17 5,e+3", "edit.paste=5,e+3", "5,3", ""},
        {"F3-18 (2+3)*4=", "edit.paste=(2+3)*4=", "20", "", "2 + 3 * 4"},
        {"F3-19 2 CRLF 3", "edit.paste=2~|3", "3", "", ""},
        {"F3-20 200 digits",
         "edit.paste=11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111",
         "1111111111111111"},
        {"F3-21 after 7+, then =", "7+ edit.paste=5 =", "12"},
        {"F3-22 :m", "4 edit.paste=:m clear memory.recall", "4"},
        {"F3-23 :r", "4 memory.store clear edit.paste=:r", "4"},
        {"F3-23 :p", "4 memory.store clear 3 edit.paste=:p clear memory.recall", "7"},
        {"F3-23 :q", "4 memory.store clear 1 edit.paste=:q clear memory.recall", "3"},
        {"F3-23 :c", "4 memory.store edit.paste=:c clear memory.recall", "0"},
        {"F3-24 :e", "4 edit.paste=:e 3=", "43"},
        {"F3-24 backslash", "4 edit.paste=\\", "4"},
        {"F3-25 @", "4 edit.paste=@", "2", "sqrt(4)"},
        {"F3-26 r", "4 edit.paste=r", "0,25", "reciproc(4)"},
        {"F3-27 q", "4 edit.paste=q", "0", ""},
        {"F3-28 s", "4 edit.paste=s", "4"},
        {"F3-29 5% after 100+", "100+ edit.paste=5%", "5", "100 + 5"},
        {"F3-29 2+3 backslash", "edit.paste=2+3\\", "3", "2 +"},
        {"F3-30 Scientific (2+3)*4=", "view.scientific edit.paste=(2+3)*4=", "20"},
        {"F3-30 Scientific 2+3*4=", "view.scientific edit.paste=2+3*4=", "14"},
        {"F3-31 Scientific s", "view.scientific 30 edit.paste=s", "0,5", "sind(30)"},
        {"F3-32 Hex ff", "view.programmer base.hexadecimal edit.paste=ff", "FF"},
        {"F3-32 Hex FF", "view.programmer base.hexadecimal edit.paste=FF", "FF"},
        {"F3-33 Hex 1G", "view.programmer base.hexadecimal edit.paste=1G", "1"},
        {"F3-33 Bin 102", "view.programmer base.binary edit.paste=102", "10"},
        {"F3-33 Dec 1,5", "view.programmer edit.paste=1,5", "15"},
        {"F3-34 Statistics 1,5", "view.statistics edit.paste=1,5", "1,5"},
        {"F3-37 Scientific q", "view.scientific 4 edit.paste=q", "0", ""},
        {"F3-37 Programmer q", "view.programmer 4 edit.paste=q", "0", ""},
        {"F3-38 2+-3=", "edit.paste=2+-3=", "-1", "", "2 - 3"},
        {"F3-39 Scientific 1e-5", "view.scientific edit.paste=1e-5", "1,e-5"},
        // Found in review: the engine's depth of 25 for brackets and pending operators, four exponent digits, E in Hex
        {"review: = with brackets the engine cannot close", "view.scientific 1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1=", "1"},
        {"review: an edit deeper than the engine takes", "view.scientific view.history 2+3= history.edit history.recalculate=1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1+2*(1)))))))))))))", "5", "", "2 + 3"},
        {"review: 30 brackets", "view.history 2+3= history.edit history.recalculate=((((((((((((((((((((((((((((((2-(3-1)))))))))))))))))))))))))))))))", "5", "", "2 + 3"},
        {"review: a five-digit exponent", "view.scientific view.history 2+3= history.edit history.recalculate=1e10000", "5", "", "2 + 3"},
        {"review: Hex E pasted", "view.programmer base.hexadecimal edit.paste=DEADBEEF", "DEADBEEF"},
        {"review: Hex E+1= pasted", "view.programmer base.hexadecimal edit.paste=E+1=", "F"},
        {"review: Hex -1E pasted", "view.programmer base.hexadecimal edit.paste=-1E", "FFFFFFFFFFFFFFE2"},
        {"review: Hex -1 2 pasted", "view.programmer base.hexadecimal edit.paste=-1_2", "FFFFFFFFFFFFFFEE"},
        {"review: Bin -1 1 pasted", "view.programmer base.binary edit.paste=-1_1", "1111111111111111111111111111111111111111111111111111111111111101"},
        {"review: Scientific -1e-5 pasted", "view.scientific edit.paste=-1e-5", "-1,e-5"},
        {"review: Hex -G5 pasted", "view.programmer base.hexadecimal edit.paste=-G5", "FFFFFFFFFFFFFFFB"},
        {"review: Standard - 5 pasted", "edit.paste=-_5", "-5", ""},
        {"review: Dec -,5 pasted", "view.programmer edit.paste=-,5", "-5"},
        {"review: Oct 9-5 pasted", "view.programmer base.octal edit.paste=9-5", "1777777777777777777773"},
        {"review: Dec 1 line end .-5 pasted", "view.programmer edit.paste=1|.-5", "-5"},
        {"review: -e-5 pasted", "edit.paste=-e-5", "-5", ""},
        {"review: -:e5 pasted", "edit.paste=-:e5", "-5", ""},
        {"review: Scientific 2E-3= pasted", "view.scientific edit.paste=2E-3=", "0,002"},
        {"review: a colon before a line end", "edit.paste=5:|3", "3", ""},
        {"review: Hex -A pasted", "view.programmer base.hexadecimal edit.paste=-A", "FFFFFFFFFFFFFFF6"},
        {"review: grouping on after Hex typed", "view.programmer base.hexadecimal FFFFFFFF view.digit_grouping", "FFFF FFFF"},
        {"review: grouping on after Oct typed", "view.programmer base.octal 1234567 view.digit_grouping", "1 234 567"},
        {"F3 during an error", "1/0= edit.paste=5", "Cannot divide by zero", "1 /"},
    };
}

// A step's text: _ is a space, | a line end, ~ a carriage return
std::wstring textOf(std::string_view written)
{
    std::wstring text;
    for (const char character : written) {
        switch (character) {
        case '_': text += L' '; break;
        case '|': text += L'\n'; break;
        case '~': text += L'\r'; break;
        default: text += static_cast<wchar_t>(static_cast<unsigned char>(character)); break;
        }
    }
    return text;
}

// Runs the steps; the copied text of the last copy command, if any
bool runSteps(chrono_calc::Calculator& calculator, const std::string& steps, std::optional<std::wstring>& copied)
{
    std::istringstream words(steps);
    std::string word;
    while (words >> word) {
        if (chrono_calc::findCommand(word)) {
            if (!calculator.run(word)) {
                return false;
            }
        } else if (const std::size_t equals = word.find('='); equals != std::string::npos && equals > 0
                                                                && chrono_calc::findCommand(word.substr(0, equals))) {
            if (!calculator.run(word.substr(0, equals), textOf(std::string_view(word).substr(equals + 1)))) {
                return false;
            }
        } else {
            for (const char character : word) {
                const std::optional<std::string_view> command =
                    chrono_calc::commandForCharacter(static_cast<unsigned char>(character), calculator.view());
                if (!command || !calculator.run(*command)) {
                    return false;
                }
            }
        }
        if (const std::optional<std::wstring> text = calculator.takeCopiedText()) {
            copied = text;
        }
    }
    return true;
}

QString historyOf(const chrono_calc::Calculator& calculator)
{
    QString joined;
    for (const std::wstring& entry : calculator.historyEntries()) {
        if (!joined.isEmpty()) {
            joined += QStringLiteral(" | ");
        }
        joined += QString::fromStdWString(entry);
    }
    return joined;
}

}  // namespace

class TestReferenceHistory : public QObject
{
    Q_OBJECT

private slots:
    static void windows7ShowsTheSame_data()
    {
        QTest::addColumn<int>("index");
        const std::vector<Line> all = lines();
        for (std::size_t index = 0; index < all.size(); ++index) {
            QTest::newRow(all[index].name) << static_cast<int>(index);
        }
    }

    static void windows7ShowsTheSame()
    {
        QFETCH(int, index);  // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        const Line line = lines()[static_cast<std::size_t>(index)];
        chrono_calc::Calculator calculator(commaSeparators);
        std::optional<std::wstring> copied;
        QVERIFY(runSteps(calculator, line.steps, copied));
        QCOMPARE(QString::fromStdWString(calculator.display()), QString::fromUtf8(line.display));
        if (line.expression != nullptr) {
            QCOMPARE(QString::fromStdWString(calculator.expression()), QString::fromUtf8(line.expression));
        }
        if (line.history != nullptr) {
            QCOMPARE(historyOf(calculator), QString::fromUtf8(line.history));
        }
        if (line.copied != nullptr) {
            QCOMPARE(QString::fromStdWString(copied.value_or(L"(nothing copied)")), QString::fromStdWString(textOf(line.copied)));
        }
    }

    // Part C line 1: the newest entry is selected; a choice selects its entry; Clear leaves none
    static void theNewestEntryIsSelected()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        std::optional<std::wstring> copied;
        QVERIFY(!calculator.selectedHistoryEntry().has_value());
        QVERIFY(runSteps(calculator, "2+3= 2*4= 1/3=", copied));
        QCOMPARE(calculator.selectedHistoryEntry(), std::optional<std::size_t>(2));
        QVERIFY(runSteps(calculator, "history.select.0", copied));
        QCOMPARE(calculator.selectedHistoryEntry(), std::optional<std::size_t>(0));
        QVERIFY(runSteps(calculator, "4+4=", copied));
        QCOMPARE(calculator.selectedHistoryEntry(), std::optional<std::size_t>(3));
        QVERIFY(runSteps(calculator, "history.clear", copied));
        QVERIFY(!calculator.selectedHistoryEntry().has_value());
    }

    // Part C line 6: Copy history of an empty history leaves the clipboard as it is
    static void copyHistoryOfNothingCopiesNothing()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("history.copy"));
        QVERIFY(!calculator.takeCopiedText().has_value());
        QVERIFY(calculator.run("view.statistics"));
        QVERIFY(calculator.run("statistics.copy_dataset"));
        QVERIFY(!calculator.takeCopiedText().has_value());
    }

    // Part D: an edit that is not taken stays open; a taken one ends; the edited entry stays selected
    static void anEditStaysOpenUntilItIsTaken()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        std::optional<std::wstring> copied;
        QVERIFY(runSteps(calculator, "view.history 1/3= 2*4= 5-1= history.select.1 history.edit", copied));
        QCOMPARE(calculator.editedHistoryEntry(), std::optional<std::size_t>(1));
        QVERIFY(runSteps(calculator, "history.recalculate=9_+*_4", copied));
        QCOMPARE(calculator.editedHistoryEntry(), std::optional<std::size_t>(1));
        QVERIFY(runSteps(calculator, "history.recalculate=52*4", copied));
        QVERIFY(!calculator.editedHistoryEntry().has_value());
        QCOMPARE(calculator.selectedHistoryEntry(), std::optional<std::size_t>(1));
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("208"));
        QCOMPARE(historyOf(calculator), QStringLiteral("1 / 3 | 52 * 4 | 5 - 1"));
    }

    // Part A line 1: View > History only works in Standard and Scientific; the setting is kept everywhere
    static void historyIsShownInStandardAndScientificOnly()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(!calculator.isHistoryShown());
        QVERIFY(calculator.isAvailable("view.history"));
        QVERIFY(calculator.run("view.history"));
        QVERIFY(calculator.isHistoryShown());
        QVERIFY(calculator.run("view.programmer"));
        QVERIFY(!calculator.isAvailable("view.history"));
        QVERIFY(calculator.run("view.history"));
        QVERIFY(calculator.isHistoryShown());
        QVERIFY(calculator.historyEntries().empty());
        QVERIFY(calculator.run("view.scientific"));
        QVERIFY(calculator.isHistoryShown());
    }

    // Part G: the Statistics view's Edit (F2)
    static void statisticsEdit_data()
    {
        QTest::addColumn<QString>("typed");
        QTest::addColumn<QString>("values");
        QTest::addColumn<bool>("stillEditing");
        QTest::newRow("G2 7") << QStringLiteral("7") << QStringLiteral("5 3 7") << false;
        QTest::newRow("G2 1,5") << QStringLiteral("1,5") << QStringLiteral("5 3 1,5") << false;
        QTest::newRow("G2 empty") << QString() << QStringLiteral("5 3") << false;
        // Windows 7 removes the value here too (part G line 3); Chrono Calc keeps the edit open (D-024)
        QTest::newRow("G3 abc") << QStringLiteral("abc") << QStringLiteral("5 3 4") << true;
    }

    static void statisticsEdit()
    {
        QFETCH(QString, typed);   // NOLINT(misc-const-correctness): QFETCH declares the variable itself
        QFETCH(QString, values);  // NOLINT(misc-const-correctness)
        QFETCH(bool, stillEditing);  // NOLINT(misc-const-correctness)
        chrono_calc::Calculator calculator(commaSeparators);
        std::optional<std::wstring> copied;
        QVERIFY(runSteps(calculator, "view.statistics 5 statistics.add 3 statistics.add 4 statistics.add statistics.edit", copied));
        QCOMPARE(calculator.editedStatisticsValue(), std::optional<std::size_t>(2));
        QVERIFY(calculator.run("statistics.apply_edit", typed.toStdWString()));
        QStringList list;
        for (const std::wstring& value : calculator.statisticsValues()) {
            list.append(QString::fromStdWString(value));
        }
        QCOMPARE(list.join(QLatin1Char(' ')), values);
        QCOMPARE(calculator.editedStatisticsValue().has_value(), stillEditing);
    }

    // Part G line 2: the first value edited keeps the order; Escape (Cancel edit) changes nothing
    static void statisticsEditKeepsTheOrder()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        std::optional<std::wstring> copied;
        QVERIFY(runSteps(calculator, "view.statistics 5 statistics.add 3 statistics.add 4 statistics.add", copied));
        QVERIFY(runSteps(calculator, "statistics.select.0 statistics.edit statistics.apply_edit=9", copied));
        QVERIFY(runSteps(calculator, "statistics.edit statistics.cancel_edit statistics.apply_edit=8", copied));
        QStringList list;
        for (const std::wstring& value : calculator.statisticsValues()) {
            list.append(QString::fromStdWString(value));
        }
        QCOMPARE(list, QStringList({QStringLiteral("9"), QStringLiteral("3"), QStringLiteral("4")}));
        QCOMPARE(calculator.selectedValue(), std::optional<std::size_t>(0));
    }

    // Part F lines 35 and 36: a line end in a pasted text is Add in the Statistics view
    static void aLineEndIsAddInStatistics()
    {
        chrono_calc::Calculator calculator(commaSeparators);
        QVERIFY(calculator.run("view.statistics"));
        QVERIFY(calculator.run("edit.paste", L"1\r\n2"));
        QCOMPARE(calculator.statisticsCount(), std::size_t{1});
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("2"));
        QVERIFY(calculator.run("edit.paste", L"\r\n3\r\n"));
        QCOMPARE(calculator.statisticsCount(), std::size_t{3});
        QCOMPARE(QString::fromStdWString(calculator.display()), QStringLiteral("0"));
        QVERIFY(calculator.run("edit.paste", L"5:\n3\n"));
        QCOMPARE(calculator.statisticsCount(), std::size_t{5});
        // A minus at the start of a line is the sign: a pasted Copy dataset keeps its values (found in review)
        QVERIFY(calculator.run("edit.paste", L"-1\n-2\n"));
        const std::vector<std::wstring> values = calculator.statisticsValues();
        QCOMPARE(values.size(), std::size_t{7});
        QCOMPARE(QString::fromStdWString(values[5]), QStringLiteral("-1"));
        QCOMPARE(QString::fromStdWString(values[6]), QStringLiteral("-2"));
    }
};

QTEST_GUILESS_MAIN(TestReferenceHistory)
#include "test_reference_history.moc"
