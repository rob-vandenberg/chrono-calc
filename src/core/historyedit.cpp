#include "core/historyedit.h"

#include "Header Files/CCommand.h"
#include "Header Files/CalcEngine.h"
#include "Ratpack/ratpak.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cwctype>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/historyedit.cpp 0.5.12

// --- Version history ----------------------------------------------------
// v0.5.12: Initial: reads an edited history entry and a typed number (Edit > History > Edit, the Statistics edit, D-024)

namespace chrono_calc {

namespace {

// The functions of the Standard and Scientific views; those with an inverse form under Inv (sin gives asind, ln powe)
constexpr std::array<std::uintptr_t, 18> functions = {IDC_SIGN, IDC_CHOP, IDC_SIN, IDC_COS, IDC_TAN, IDC_SINH, IDC_COSH,
                                                     IDC_TANH, IDC_LN, IDC_LOG, IDC_SQRT, IDC_SQR, IDC_CUB, IDC_FAC,
                                                     IDC_REC, IDC_DMS, IDC_CUBEROOT, IDC_POW10};
constexpr std::array<std::uintptr_t, 7> operators = {IDC_ADD, IDC_SUB, IDC_MUL, IDC_DIV, IDC_PWR, IDC_MOD, IDC_ROOT};

bool hasInverse(std::uintptr_t opCode)
{
    switch (opCode) {
    case IDC_SIN:
    case IDC_COS:
    case IDC_TAN:
    case IDC_SINH:
    case IDC_COSH:
    case IDC_TANH:
    case IDC_LN:
    case IDC_CHOP:
    case IDC_DMS:
        return true;
    default:
        return false;
    }
}

bool isTrigonometric(std::uintptr_t opCode)
{
    return opCode == IDC_SIN || opCode == IDC_COS || opCode == IDC_TAN;
}

bool sameWord(std::wstring_view first, std::wstring_view second)
{
    return first.size() == second.size()
           && std::ranges::equal(first, second, [](wchar_t one, wchar_t other) { return std::towlower(one) == std::towlower(other); });
}

// The step of one function as the engine writes it ("sqrt", "sind", "asinr", "powe"), or nothing where it has no
// such form
std::optional<EditStep> functionStep(std::uintptr_t opCode, bool inverse, AngleType angle, std::uintptr_t angleCommand)
{
    const std::wstring_view written = CCalcEngine::OpCodeToUnaryString(static_cast<int>(opCode), inverse, angle);
    if (written.empty()) {
        return std::nullopt;
    }
    EditStep step{.kind = EditStep::Kind::Function, .number = {}, .commands = {}, .text = std::wstring(written)};
    if (isTrigonometric(opCode)) {
        step.commands.push_back(angleCommand);
    }
    if (inverse) {
        step.commands.push_back(IDC_INV);
    }
    step.commands.push_back(opCode);
    return step;
}

// Every function by the name the engine writes it with: each form under Inv, and sin, cos and tan in each angle unit
std::vector<EditStep> functionSteps()
{
    constexpr std::array<std::pair<AngleType, std::uintptr_t>, 3> angles = {
        std::pair{AngleType::Degrees, static_cast<std::uintptr_t>(IDC_DEG)},
        std::pair{AngleType::Radians, static_cast<std::uintptr_t>(IDC_RAD)},
        std::pair{AngleType::Gradians, static_cast<std::uintptr_t>(IDC_GRAD)}};
    std::vector<EditStep> steps;
    for (const std::uintptr_t opCode : functions) {
        for (const bool inverse : {false, true}) {
            const std::size_t angleCount = isTrigonometric(opCode) ? angles.size() : 1;
            for (std::size_t angle = 0; angle < angleCount && (!inverse || hasInverse(opCode)); ++angle) {
                if (std::optional<EditStep> step = functionStep(opCode, inverse, angles.at(angle).first, angles.at(angle).second)) {
                    steps.push_back(std::move(*step));
                }
            }
        }
    }
    return steps;
}

std::optional<EditStep> functionNamed(std::wstring_view name)
{
    for (EditStep& step : functionSteps()) {
        if (sameWord(step.text, name)) {
            return std::move(step);
        }
    }
    return std::nullopt;
}

// The operator written this way ("+", "Mod", "yroot")
std::optional<EditStep> operatorNamed(std::wstring_view name)
{
    for (const std::uintptr_t opCode : operators) {
        const std::wstring_view written = CCalcEngine::OpCodeToBinaryString(static_cast<int>(opCode), false);
        if (!written.empty() && sameWord(written, name)) {
            return EditStep{.kind = EditStep::Kind::Operator, .number = {}, .commands = {opCode}, .text = std::wstring(written)};
        }
    }
    return std::nullopt;
}

class Reader
{
public:
    Reader(std::wstring_view text, const NumberRules& rules)
        : m_text(text)
        , m_rules(rules)
    {
    }

    // The steps of the whole text: operands (a number, a signed number, a bracket, a function with its bracket)
    // with operators between them, every bracket closed
    std::optional<std::vector<EditStep>> steps()
    {
        std::vector<EditStep> steps;
        std::size_t openBrackets = 0;
        bool operandNext = true;
        while (true) {
            skipSpaces();
            if (operandNext) {
                std::optional<EditStep> step = readOperand();
                if (!step) {
                    return std::nullopt;
                }
                const bool opens = step->kind != EditStep::Kind::Number;
                openBrackets += opens ? 1 : 0;
                operandNext = opens;
                steps.push_back(std::move(*step));
                continue;
            }
            if (atEnd()) {
                break;
            }
            if (m_text[m_position] == L')') {
                if (openBrackets == 0) {
                    return std::nullopt;
                }
                ++m_position;
                --openBrackets;
                steps.push_back(EditStep{.kind = EditStep::Kind::Close, .number = {}, .commands = {}, .text = L")"});
                continue;
            }
            std::optional<EditStep> found = readOperator();
            if (!found) {
                return std::nullopt;
            }
            steps.push_back(std::move(*found));
            operandNext = true;
        }
        if (openBrackets != 0) {
            return std::nullopt;
        }
        return steps;
    }

    // An opening bracket, a function's name with its bracket, or a number
    std::optional<EditStep> readOperand()
    {
        if (atEnd()) {
            return std::nullopt;
        }
        const wchar_t character = m_text[m_position];
        if (character == L'(') {
            ++m_position;
            return EditStep{.kind = EditStep::Kind::Open, .number = {}, .commands = {}, .text = L"("};
        }
        if (std::iswalpha(character) != 0) {
            std::optional<EditStep> function = functionNamed(readWord());
            skipSpaces();
            if (!function || atEnd() || m_text[m_position] != L'(') {
                return std::nullopt;
            }
            ++m_position;
            return function;
        }
        std::optional<TypedNumber> number = readNumberHere();
        if (!number) {
            return std::nullopt;
        }
        return EditStep{.kind = EditStep::Kind::Number, .number = std::move(*number), .commands = {}, .text = {}};
    }

    std::optional<EditStep> readOperator()
    {
        if (atEnd()) {
            return std::nullopt;
        }
        if (std::iswalpha(m_text[m_position]) != 0) {
            return operatorNamed(readWord());
        }
        const std::optional<EditStep> found = operatorNamed(m_text.substr(m_position, 1));
        if (found) {
            ++m_position;
        }
        return found;
    }

    // [-] digits [decimal digits] [e [+-] digits]
    std::optional<TypedNumber> readNumberHere()
    {
        TypedNumber number;
        if (!atEnd() && m_text[m_position] == L'-') {
            number.negative = true;
            ++m_position;
        }
        if (!readMantissa(number) || !readExponent(number)) {
            return std::nullopt;
        }
        return number;
    }

    // The digits and the decimal point; the thousands symbol is skipped (Windows 7: "1.5" is 15 where the decimal
    // symbol is a comma); at most the view's number of digits, leading zeros not counted
    bool readMantissa(TypedNumber& number)
    {
        bool decimal = false;
        bool leading = true;
        std::size_t digits = 0;
        std::size_t typed = 0;
        for (; !atEnd(); ++m_position) {
            const wchar_t character = m_text[m_position];
            if (character >= L'0' && character <= L'9') {
                number.mantissa += character;
                ++typed;
                leading = leading && character == L'0' && !decimal;
                digits += leading ? 0 : 1;
            } else if (character == m_rules.decimal && !decimal) {
                decimal = true;
                number.mantissa += L'.';
            } else if (character != m_rules.thousands || m_rules.thousands == L' ') {
                break;
            }
        }
        return typed > 0 && digits <= m_rules.mostDigits;
    }

    // "e", its sign and its digits, where an "e" follows the digits
    bool readExponent(TypedNumber& number)
    {
        if (atEnd() || (m_text[m_position] != L'e' && m_text[m_position] != L'E')) {
            return true;
        }
        ++m_position;
        number.hasExponent = true;
        if (!atEnd() && (m_text[m_position] == L'+' || m_text[m_position] == L'-')) {
            number.exponentNegative = m_text[m_position] == L'-';
            ++m_position;
        }
        for (; !atEnd() && m_text[m_position] >= L'0' && m_text[m_position] <= L'9'; ++m_position) {
            if (!number.exponent.empty() || m_text[m_position] != L'0') {
                number.exponent += m_text[m_position];
            }
        }
        // The engine takes four digits of an exponent and drops the rest without a word (scidisp.cpp MAX_EXPONENT):
        // a longer one is no number it can take; leading zeros do not count
        constexpr std::size_t mostExponentDigits = 4;
        if (number.exponent.empty() && m_text[m_position - 1] == L'0') {
            number.exponent = L"0";
        }
        return !number.exponent.empty() && number.exponent.size() <= mostExponentDigits;
    }

    void skipSpaces()
    {
        while (!atEnd() && std::iswspace(m_text[m_position]) != 0) {
            ++m_position;
        }
    }

    [[nodiscard]] bool atEnd() const
    {
        return m_position >= m_text.size();
    }

private:
    std::wstring_view readWord()
    {
        const std::size_t start = m_position;
        while (!atEnd() && std::iswalpha(m_text[m_position]) != 0) {
            ++m_position;
        }
        return m_text.substr(start, m_position - start);
    }

    std::wstring_view m_text;
    NumberRules m_rules;
    std::size_t m_position = 0;
};

}  // namespace

std::optional<std::vector<EditStep>> readEdit(std::wstring_view text, const NumberRules& rules)
{
    Reader reader(text, rules);
    return reader.steps();
}

std::optional<TypedNumber> readNumber(std::wstring_view text, const NumberRules& rules)
{
    Reader reader(text, rules);
    reader.skipSpaces();
    std::optional<TypedNumber> number = reader.readNumberHere();
    reader.skipSpaces();
    if (!number || !reader.atEnd()) {
        return std::nullopt;
    }
    return number;
}

}  // namespace chrono_calc
