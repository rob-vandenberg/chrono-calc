// The engine texts below the line "Microsoft's" are the en-US texts of microsoft/calculator commit 79e1ac5,
// src/Calculator/Resources/en-US/CEngineStrings.resw. Copyright (c) Microsoft Corporation. Licensed under the MIT License.

#include "core/enginetexts.h"

#include <string>
#include <string_view>
#include <unordered_map>

// --- Version ------------------------------------------------------------
// src/core/enginetexts.cpp 0.2.7

// --- Version history ----------------------------------------------------
// v0.2.7: Windows 7's names of the Scientific functions (sind, asind, sinr, sing, powe, powten) and of * and /; no
//         digit grouping, as Windows 7 starts
// v0.1.5: Windows 7's names in the expression line (sqrt, reciproc); the separators come from the caller
// v0.0.4: Initial: a text for every text number the engine can look up (D-015)

namespace chrono_calc {

namespace {

const std::unordered_map<std::wstring_view, std::wstring_view>& texts()
{
    static const std::unordered_map<std::wstring_view, std::wstring_view> table = {
        // Microsoft's
        {L"10", L"Rsh"},
        {L"100", L"Invalid input"},
        {L"101", L"Result is undefined"},
        {L"105", L"Not enough memory"},
        {L"107", L"Overflow"},
        {L"108", L"Result not defined"},
        {L"11", L"/"},  // Windows 7 (Microsoft's: ÷)
        {L"118", L"Result not defined"},
        {L"119", L"Overflow"},
        {L"12", L"*"},  // Windows 7 (Microsoft's: ×)
        {L"120", L"Overflow"},
        {L"13", L"+"},
        {L"14", L"-"},
        {L"15", L"Mod"},
        {L"16", L"yroot"},
        {L"17", L"^"},
        {L"18", L"Int"},
        {L"19", L"RoL"},
        {L"2", L"CE"},
        {L"20", L"RoR"},
        {L"21", L"NOT"},
        {L"22", L"sin"},
        {L"23", L"cis"},
        {L"24", L"tan"},
        {L"25", L"sinh"},
        {L"26", L"cosh"},
        {L"27", L"tanh"},
        {L"28", L"ln"},
        {L"29", L"log"},
        {L"30", L"sqrt"},  // Windows 7 (Microsoft's: √)
        {L"35", L"dms"},
        {L"37", L"powten"},  // Windows 7
        {L"38", L"%"},
        {L"4", L"."},
        {L"40", L"Pi"},
        {L"41", L"="},
        {L"47", L"Exp"},
        {L"48", L"("},
        {L"49", L")"},
        {L"6", L"AND"},
        {L"66", L"frac"},
        {L"67", L"sind"},  // Windows 7
        {L"68", L"cosd"},  // Windows 7
        {L"69", L"tand"},  // Windows 7
        {L"7", L"OR"},
        {L"70", L"asind"},  // Windows 7
        {L"71", L"acosd"},  // Windows 7
        {L"72", L"atand"},  // Windows 7
        {L"73", L"sinr"},  // Windows 7
        {L"74", L"cosr"},  // Windows 7
        {L"75", L"tanr"},  // Windows 7
        {L"76", L"asinr"},  // Windows 7
        {L"77", L"acosr"},  // Windows 7
        {L"78", L"atanr"},  // Windows 7
        {L"79", L"sing"},  // Windows 7
        {L"8", L"XOR"},
        {L"80", L"cosg"},  // Windows 7
        {L"81", L"tang"},  // Windows 7
        {L"82", L"asing"},  // Windows 7
        {L"83", L"acosg"},  // Windows 7
        {L"84", L"atang"},  // Windows 7
        {L"85", L"asinh"},  // Windows 7
        {L"86", L"acosh"},  // Windows 7
        {L"87", L"atanh"},  // Windows 7
        {L"88", L"powe"},  // Windows 7
        {L"89", L"powten"},  // Windows 7
        {L"9", L"Lsh"},
        {L"90", L"sqrt"},  // Windows 7 (Microsoft's: √)
        {L"91", L"sqr"},
        {L"92", L"cube"},
        {L"94", L"fact"},
        {L"95", L"reciproc"},  // Windows 7 (Microsoft's: 1/)
        {L"96", L"degrees"},
        {L"97", L"negate"},
        {L"99", L"Cannot divide by zero"},
        {L"SecDeg", L"sec₀"},
        {L"SecRad", L"secᵣ"},
        {L"SecGrad", L"sec₉"},
        {L"InverseSecDeg", L"sec₀⁻¹"},
        {L"InverseSecRad", L"secᵣ⁻¹"},
        {L"InverseSecGrad", L"sec₉⁻¹"},
        {L"CscDeg", L"csc₀"},
        {L"CscRad", L"cscᵣ"},
        {L"CscGrad", L"csc₉"},
        {L"InverseCscDeg", L"csc₀⁻¹"},
        {L"InverseCscRad", L"cscᵣ⁻¹"},
        {L"InverseCscGrad", L"csc₉⁻¹"},
        {L"CotDeg", L"cot₀"},
        {L"CotRad", L"cotᵣ"},
        {L"CotGrad", L"cot₉"},
        {L"InverseCotDeg", L"cot₀⁻¹"},
        {L"InverseCotRad", L"cotᵣ⁻¹"},
        {L"InverseCotGrad", L"cot₉⁻¹"},
        {L"Sech", L"sech"},
        {L"InverseSech", L"sech⁻¹"},
        {L"Csch", L"csch"},
        {L"InverseCsch", L"csch⁻¹"},
        {L"Coth", L"coth"},
        {L"InverseCoth", L"coth⁻¹"},
        {L"TwoPowX", L"2^"},
        {L"LogBaseY", L"log base"},
        {L"Abs", L"abs"},
        {L"Ceil", L"ceil"},
        {L"Floor", L"floor"},
        {L"Nand", L"NAND"},
        {L"Nor", L"NOR"},
        {L"CubeRoot", L"cuberoot"},
        {L"ProgrammerMod", L"%"},
        // Ours: texts the engine can ask for that CEngineStrings.resw does not hold; to be checked against Windows 7 (task 1.5)
        {L"0", L"±"},
        {L"3", L"←"},
        {L"31", L"x²"},
        {L"32", L"x³"},
        {L"33", L"n!"},
        {L"34", L"1/x"},
        {L"39", L"F-E"},
        {L"42", L"MC"},
        {L"43", L"MR"},
        {L"44", L"MS"},
        {L"45", L"M+"},
        {L"46", L"M-"},
        {L"50", L"0"},
        {L"51", L"1"},
        {L"52", L"2"},
        {L"53", L"3"},
        {L"54", L"4"},
        {L"55", L"5"},
        {L"56", L"6"},
        {L"57", L"7"},
        {L"58", L"8"},
        {L"59", L"9"},
        {L"60", L"A"},
        {L"61", L"B"},
        {L"62", L"C"},
        {L"63", L"D"},
        {L"64", L"E"},
        {L"65", L"F"},
        {L"93", L"cuberoot"},
        {L"102", L"Overflow"},
        {L"103", L"Overflow"},
        {L"104", L"Calculation aborted"},
        {L"106", L"Too many items"},
        {L"109", L"Insufficient data"},
        {L"111", L"Invalid input"},
        {L"112", L"Invalid input"},
        {L"113", L"Invalid input"},
        {L"114", L"Invalid input"},
        {L"115", L"Invalid input"},
        {L"116", L"Invalid input"},
        {L"117", L"Invalid input"},
    };
    return table;
}

}  // namespace

EngineTexts::EngineTexts(Separators separators)
    : m_separators(separators)
{
}

void EngineTexts::setSeparators(Separators separators)
{
    m_separators = separators;
}

std::wstring EngineTexts::GetCEngineString(std::wstring_view id)
{
    if (id == L"sDecimal") {
        return {m_separators.decimal};
    }
    if (id == L"sThousand") {
        return {m_separators.thousands};
    }
    if (id == L"sGrouping") {
        return L"0;0";  // no digit grouping: Windows 7 starts with View > Digit grouping off (1234, not 1,234)
    }
    const auto found = texts().find(id);
    return found == texts().end() ? std::wstring() : std::wstring(found->second);
}

}  // namespace chrono_calc
