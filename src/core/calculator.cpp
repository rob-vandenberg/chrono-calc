#include "core/calculator.h"

#include "core/commands.h"
#include "core/enginetexts.h"
#include "core/statistics.h"
#include "core/view.h"

#include "CalcManager/CalculatorHistory.h"
#include "CalcManager/Command.h"
#include "CalcManager/ExpressionCommandInterface.h"
#include "Header Files/CCommand.h"
#include "Header Files/CalcEngine.h"
#include "Header Files/ICalcDisplay.h"
#include "Header Files/Rational.h"
#include "Ratpack/CalcErr.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/calculator.cpp 0.4.11

// --- Version history ----------------------------------------------------
// v0.4.11: The Statistics view: a fourth engine (16 digits, as Windows 7), the list of values with its selection, Add,
//          CAD, the statistics computed on the engine's exact fractions (D-010, D-023)
// v0.3.8: The Programmer view: a third engine in integer mode (64 bits); bases and word sizes kept over a view change;
//         the bits; which commands each view and base allows (D-021)
// v0.2.7: The Scientific view: its own engine with precedence and 32 digits, the one memory shared; Inv, the angle
//         units, F-E, brackets; the expression line from the engine as Windows 7 shows it (D-020)
// v0.1.6: view.standard and view.basic change nothing while the calculator is already in Standard and Basic, as in
//         Windows 7; percent right after = takes the result: 2*3=5% is 6 * 5/100 = 0.3 (Windows 7)
// v0.1.5: Drives the engine directly (its one memory register is Windows 7's memory); Windows 7's expression line and
//         percent; the separators of the regional settings (D-017)
// v0.0.4: Initial: the engine wrapper; runs commands and keeps what the engine shows (D-015)

namespace chrono_calc {

namespace {

// Windows 10's Standard precision, which Windows 7 shows as well (16 digits: 1/3 = 0.3333333333333333)
constexpr int32_t standardPrecision = 16;
// The Scientific view shows 32 digits, in Windows 7 as in Windows 10
constexpr int32_t scientificPrecision = 32;
// The Programmer view works on 64-bit integers (Windows 10's precision for it)
constexpr int32_t programmerPrecision = 64;
constexpr std::size_t bitCount = 64;
constexpr std::size_t historySize = 20;
// Windows 7's text when a statistic has too few values (none, or one for σn-1); the engine has no such text
constexpr std::wstring_view enterDataText = L"Enter data to calculate";

bool isBinaryOperator(std::uintptr_t opCode)
{
    return opCode == IDC_ADD || opCode == IDC_SUB || opCode == IDC_MUL || opCode == IDC_DIV;
}

// Windows 7 writes the operators of the expression line as plain characters
wchar_t operatorCharacter(std::uintptr_t opCode)
{
    switch (opCode) {
    case IDC_ADD: return L'+';
    case IDC_SUB: return L'-';
    case IDC_MUL: return L'*';
    default: return L'/';
    }
}

// The functions that have an inverse form under Inv in Windows 7 (sin gives sin⁻¹, ln gives eˣ, Int gives Frac, dms
// gives deg, pi gives 2*pi); the engine gives that form itself when its Inv is set, and resets its Inv afterwards
bool hasInverseForm(std::uintptr_t opCode)
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
    case IDC_PI:
        return true;
    default:
        return false;
    }
}

// The commands only the Scientific view has
bool isScientificOnly(std::uintptr_t opCode)
{
    switch (opCode) {
    case IDC_SIN:
    case IDC_COS:
    case IDC_TAN:
    case IDC_SINH:
    case IDC_COSH:
    case IDC_TANH:
    case IDC_LN:
    case IDC_LOG:
    case IDC_POW10:
    case IDC_SQR:
    case IDC_CUB:
    case IDC_CUBEROOT:
    case IDC_PWR:
    case IDC_ROOT:
    case IDC_FAC:
    case IDC_CHOP:
    case IDC_DMS:
    case IDC_PI:
    case IDC_FE:
    case IDC_EXP:
    case IDC_MOD:
    case IDC_OPENP:
    case IDC_CLOSEP:
    case IDC_DEG:
    case IDC_RAD:
    case IDC_GRAD:
        return true;
    default:
        return false;
    }
}

// The commands only the Programmer view has
bool isProgrammerOnly(std::uintptr_t opCode)
{
    return (opCode >= IDC_A && opCode <= IDC_F) || opCode == IDC_AND || opCode == IDC_OR || opCode == IDC_XOR || opCode == IDC_COM
           || opCode == IDC_LSHF || opCode == IDC_RSHF || opCode == IDC_ROL || opCode == IDC_ROR || (opCode >= IDM_HEX && opCode <= IDM_BIN)
           || (opCode >= IDM_QWORD && opCode <= IDM_BYTE) || (opCode >= IDC_BINEDITSTART && opCode <= IDC_BINEDITEND);
}

// How many digits a base has
std::uintptr_t digitCount(NumberBase base)
{
    switch (base) {
    case NumberBase::Hexadecimal: return 16;
    case NumberBase::Octal: return 8;
    case NumberBase::Binary: return 2;
    case NumberBase::Decimal: break;
    }
    return 10;
}

// What the Programmer view allows (Windows 7): the shared keys without the decimal point, sqrt, % and 1/x; the
// Scientific Mod and brackets; its own commands; and only the digits of the current base
bool programmerAllows(std::uintptr_t opCode, NumberBase base)
{
    if (opCode == IDC_PNT || opCode == IDC_SQRT || opCode == IDC_PERCENT || opCode == IDC_REC) {
        return false;
    }
    if (opCode >= IDC_0 && opCode <= IDC_F) {
        return opCode - IDC_0 < digitCount(base);
    }
    if (isScientificOnly(opCode)) {
        return opCode == IDC_MOD || opCode == IDC_OPENP || opCode == IDC_CLOSEP;
    }
    return true;
}

// Windows 7 writes brackets tight against what they hold: "(2 +", "((", "(2 + 3)"
std::wstring tightBrackets(const std::wstring& text)
{
    std::wstring result;
    for (const wchar_t character : text) {
        if (character == L' ' && !result.empty() && result.back() == L'(') {
            continue;
        }
        if (character == L')' && !result.empty() && result.back() == L' ') {
            result.pop_back();
        }
        result += character;
    }
    return result;
}

// What the Statistics view has (Windows 7, windows7-statistics.md part A): the digits, the decimal point, backspace, C,
// plus/minus, the memory, F-E and Exp; no operators, no CE
bool statisticsAllows(std::uintptr_t opCode)
{
    switch (opCode) {
    case IDC_PNT:
    case IDC_BACK:
    case IDC_CLEAR:
    case IDC_SIGN:
    case IDC_MCLEAR:
    case IDC_RECALL:
    case IDC_STORE:
    case IDC_MPLUS:
    case IDC_MMINUS:
    case IDC_FE:
    case IDC_EXP:
        return true;
    default:
        return opCode >= IDC_0 && opCode <= IDC_9;
    }
}

bool isStatisticsCommand(CommandKind kind)
{
    switch (kind) {
    case CommandKind::StatisticsAdd:
    case CommandKind::StatisticsClearAll:
    case CommandKind::StatisticsFunction:
    case CommandKind::StatisticsSelect:
    case CommandKind::StatisticsSelectPrevious:
    case CommandKind::StatisticsSelectNext:
    case CommandKind::StatisticsDelete:
        return true;
    default:
        return false;
    }
}

// Whether a statistic has enough values: every statistic needs one, σn-1 two (Windows 7 shows "Enter data to calculate"
// otherwise, also for Σx of an empty list)
bool hasEnoughValues(Statistic statistic, std::size_t count)
{
    return count >= (statistic == Statistic::SampleDeviation ? 2U : 1U);
}

// The precision of each view's engine
int32_t precisionOf(View view)
{
    switch (view) {
    case View::Scientific: return scientificPrecision;
    case View::Programmer: return programmerPrecision;
    case View::Standard:
    case View::Statistics: break;
    }
    return standardPrecision;
}

std::wstring trimmed(const std::wstring& text)
{
    const auto first = text.find_first_not_of(L' ');
    if (first == std::wstring::npos) {
        return {};
    }
    return text.substr(first, text.find_last_not_of(L' ') - first + 1);
}

}  // namespace

// Receives what the engine reports and keeps it as plain values
class Calculator::Display : public ICalcDisplay
{
public:
    Display() = default;
    virtual ~Display() = default;
    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;
    Display(Display&&) = delete;
    Display& operator=(Display&&) = delete;

    void SetPrimaryDisplay(const std::wstring& text, bool isError) override
    {
        m_primary = text;
        m_isError = isError;
    }

    void SetIsInError(bool isInError) override
    {
        m_isError = isInError;
    }

    // Keeps what follows the engine's last binary operator: the current value's own part ("sqrt(9)", "0.06")
    void SetExpressionDisplay(const std::shared_ptr<std::vector<std::pair<std::wstring, int>>>& tokens,
                              const std::shared_ptr<std::vector<std::shared_ptr<IExpressionCommand>>>& commands) override
    {
        m_tail.clear();
        m_full.clear();
        if (!tokens) {
            return;
        }
        std::wstring full;
        for (const auto& token : *tokens) {
            full += token.first;
        }
        m_full = trimmed(full);
        std::size_t start = 0;
        for (std::size_t index = 0; index < tokens->size(); ++index) {
            const int command = (*tokens)[index].second;
            if (commands && command >= 0 && static_cast<std::size_t>(command) < commands->size()
                && (*commands)[static_cast<std::size_t>(command)]->GetCommandType() == CalculationManager::CommandType::BinaryCommand) {
                start = index + 1;
            }
        }
        std::wstring tail;
        for (std::size_t index = start; index < tokens->size(); ++index) {
            tail += (*tokens)[index].first;
        }
        m_tail = trimmed(tail);
    }

    void SetParenthesisNumber(unsigned int count) override
    {
        m_openBrackets = count;
    }

    // Reports the window does not use
    void SetMemorizedNumbers(const std::vector<std::wstring>& /*memorizedNumbers*/) override {}
    void OnNoRightParenAdded() override {}
    void MaxDigitsReached() override {}
    void BinaryOperatorReceived() override {}
    void OnHistoryItemAdded(unsigned int /*addedItemIndex*/) override {}
    void MemoryItemChanged(unsigned int /*indexOfMemory*/) override {}
    void InputChanged() override {}

    [[nodiscard]] const std::wstring& primary() const
    {
        return m_primary;
    }

    [[nodiscard]] const std::wstring& tail() const
    {
        return m_tail;
    }

    [[nodiscard]] bool isError() const
    {
        return m_isError;
    }

    // The engine's whole expression, as its tokens give it ("2 + sind(30)")
    [[nodiscard]] const std::wstring& full() const
    {
        return m_full;
    }

    [[nodiscard]] unsigned int openBrackets() const
    {
        return m_openBrackets;
    }

private:
    std::wstring m_primary;  // empty until the engine sends its first number
    std::wstring m_tail;
    std::wstring m_full;
    bool m_isError = false;
    unsigned int m_openBrackets = 0;
};

namespace {

std::unique_ptr<CCalcEngine> newEngine(bool precedence, EngineTexts& texts, ICalcDisplay* display,
                                       const std::shared_ptr<CalculationManager::CalculatorHistory>& history, bool integers = false)
{
    CCalcEngine::InitialOneTimeOnlySetup(texts);  // the engine's texts and number constants; the same for every engine
    return std::make_unique<CCalcEngine>(precedence, integers, &texts, display, history);
}

}  // namespace

Calculator::Calculator(Separators separators)
    : m_texts(separators)
    , m_display(std::make_unique<Display>())
    , m_standardHistory(std::make_shared<CalculationManager::CalculatorHistory>(historySize))
    , m_scientificHistory(std::make_shared<CalculationManager::CalculatorHistory>(historySize))
    , m_programmerHistory(std::make_shared<CalculationManager::CalculatorHistory>(historySize))
    , m_statisticsHistory(std::make_shared<CalculationManager::CalculatorHistory>(historySize))
    , m_standardEngine(newEngine(false /* left to right */, m_texts, m_display.get(), m_standardHistory))
    , m_scientificEngine(newEngine(true /* precedence */, m_texts, m_display.get(), m_scientificHistory))
    , m_programmerEngine(newEngine(true /* precedence */, m_texts, m_display.get(), m_programmerHistory, true /* integers */))
    , m_statisticsEngine(newEngine(false /* no operators */, m_texts, m_display.get(), m_statisticsHistory))
    , m_engine(m_standardEngine.get())
{
    // The engine never gives m_decimalSeparator a start value (CalcEngine.h:166), and SettingsChanged() only updates the
    // decimal symbol of its input when the new one differs from that value: by chance it may not, and then "0,1" is read
    // as an overflow. Reporting another separator once and then the real one makes the engine update it in every case.
    m_texts.setSeparators(Separators{.decimal = separators.decimal == L'.' ? L',' : L'.', .thousands = separators.thousands});
    for (CCalcEngine* engine : {m_statisticsEngine.get(), m_programmerEngine.get(), m_scientificEngine.get(), m_standardEngine.get()}) {
        engine->SettingsChanged();
    }
    m_texts.setSeparators(separators);
    // The Standard engine is set up last: the number library's shared constants follow the last engine's precision, and
    // Standard is the view the calculator starts in
    for (const View view : {View::Statistics, View::Programmer, View::Scientific, View::Standard}) {
        CCalcEngine* engine = engineOf(view);
        engine->SettingsChanged();
        engine->ProcessCommand(IDC_DEC);
        engine->ChangePrecision(precisionOf(view));
        engine->UpdateMaxIntDigits();
        engine->ProcessCommand(IDC_CLEAR);
    }
    // The one memory starts at 0 (a recall from a memory that was never set would read nothing); it lives in the
    // engine of the current view and moves with a view change
    m_engine->PersistedMemObject(CalcEngine::Rational(0));
    m_engine->ProcessCommand(IDC_CLEAR);
}

// The engine points to the display, the texts and the history; it is declared last, so it is destroyed first
Calculator::~Calculator() = default;

bool Calculator::run(std::string_view commandId)
{
    const auto target = findCommand(commandId);
    if (!target) {
        return false;
    }
    // Basic only hides a side panel. Choosing the current view again changes nothing (Windows 7).
    switch (target->kind) {
    case CommandKind::ViewBasic:
        return true;
    case CommandKind::ViewStandard:
        switchView(View::Standard);
        return true;
    case CommandKind::ViewScientific:
        switchView(View::Scientific);
        return true;
    case CommandKind::ViewProgrammer:
        switchView(View::Programmer);
        return true;
    case CommandKind::ViewStatistics:
        switchView(View::Statistics);
        return true;
    case CommandKind::StatisticsAdd:
    case CommandKind::StatisticsClearAll:
    case CommandKind::StatisticsFunction:
    case CommandKind::StatisticsSelect:
    case CommandKind::StatisticsSelectPrevious:
    case CommandKind::StatisticsSelectNext:
    case CommandKind::StatisticsDelete:
        if (m_view == View::Statistics) {
            runStatisticsCommand(*target);
        }
        return true;
    case CommandKind::Inverse:
        // Not while an error shows: the engine then takes only C and CE
        if (m_view == View::Scientific && !isError()) {
            m_inverse = !m_inverse;
        }
        return true;
    case CommandKind::Engine:
        break;
    }
    const auto opCode = static_cast<std::uintptr_t>(target->engineCommand);
    // While an error shows, the engine takes only C and CE (scicomm.cpp); every other key changes nothing, so neither
    // the switches nor the expression line change either (passing it on would make the engine empty its expression)
    if (isError() && opCode != IDC_CLEAR && opCode != IDC_CENTR) {
        return true;
    }
    // A command the view does not have, or greys (Windows 7), does nothing: a key without a button there
    if (!isAvailable(commandId)) {
        return true;
    }
    if (m_view == View::Scientific) {
        runScientific(opCode);
        return true;
    }
    if (m_view == View::Programmer) {
        runProgrammer(opCode);
        return true;
    }
    if (m_view == View::Statistics) {
        runStatistics(opCode);
        return true;
    }
    if (isBinaryOperator(opCode)) {
        binaryOperator(opCode);
        return true;
    }
    if (opCode == IDC_PERCENT) {
        percent();
        return true;
    }
    // While an error shows, the engine takes CE as C (scicomm.cpp), so the expression line is cleared as well
    const bool clears = opCode == IDC_CLEAR || (opCode == IDC_CENTR && isError());
    runEngine(opCode);
    if (clears) {
        m_chain.clear();
        m_pendingOperator = 0;
        m_leftValue.reset();
        m_lastWasOperator = false;
        m_percentText.clear();
        m_afterEquals = false;
        m_result.reset();
        return true;
    }
    switch (opCode) {
    case IDC_EQU:
        // Windows 7 empties the expression line after =, but keeps it when = ends in an error ("1 /")
        if (!isError()) {
            m_chain.clear();
        }
        m_pendingOperator = 0;
        m_leftValue.reset();
        m_lastWasOperator = false;
        m_percentText.clear();
        m_afterEquals = true;
        // A percent right after = takes this result as its base (Windows 7)
        if (!isError()) {
            m_result = currentValue();
        }
        break;
    case IDC_BACK:  // backspace only changes a number being typed; on a result or after an operator it changes nothing
    case IDC_STORE:  // the memory changes nothing on the display
    case IDC_MCLEAR:
    case IDC_MPLUS:
    case IDC_MMINUS:
        break;
    default:
        // A new or changed current value (a digit, CE, backspace, plus/minus, sqrt, reciprocal, MR)
        m_lastWasOperator = false;
        m_percentText.clear();
        m_afterEquals = false;
        break;
    }
    return true;
}

std::wstring Calculator::display() const
{
    if (!m_statisticsError.empty()) {
        return m_statisticsError;
    }
    // The engine always groups hexadecimal, octal and binary numbers with spaces (scidisp.cpp, GroupDigitsPerRadix:
    // Windows 10's choice); Windows 7 starts with Digit grouping off and shows them whole, as it does decimals
    if (m_view == View::Programmer && m_numberBase != NumberBase::Decimal && !isError()) {
        std::wstring whole = m_display->primary();
        std::erase(whole, L' ');
        return whole;
    }
    return m_display->primary();
}

std::wstring Calculator::expression() const
{
    if (m_view == View::Programmer || m_view == View::Statistics) {
        return {};  // Windows 7 shows no expression line in the Programmer and Statistics views
    }
    if (m_view == View::Scientific) {
        // The engine's own expression with Windows 7's names; empty after =, but kept when = ends in an error ("1 /")
        return m_afterEquals ? std::wstring() : tightBrackets(m_display->full());
    }
    std::wstring current = m_percentText.empty() ? m_display->tail() : m_percentText;
    if (m_lastWasOperator || m_afterEquals) {
        current.clear();
    }
    if (m_chain.empty()) {
        return current;
    }
    return current.empty() ? m_chain : m_chain + L' ' + current;
}

bool Calculator::isError() const
{
    return m_display->isError() || !m_statisticsError.empty();
}

bool Calculator::hasMemory()
{
    return memoryValue() != CalcEngine::Rational(0);
}

View Calculator::view() const
{
    return m_view;
}

bool Calculator::isInverse() const
{
    return m_inverse;
}

AngleUnit Calculator::angleUnit() const
{
    return m_angleUnit;
}

bool Calculator::isScientificNotation() const
{
    return m_scientificNotation;
}

unsigned int Calculator::openBrackets() const
{
    return m_display->openBrackets();
}

bool Calculator::isAvailable(View view, NumberBase base, std::string_view commandId)
{
    const auto target = findCommand(commandId);
    if (!target) {
        return false;
    }
    if (target->kind == CommandKind::Inverse) {
        return view == View::Scientific;
    }
    if (isStatisticsCommand(target->kind)) {
        return view == View::Statistics;
    }
    if (target->kind != CommandKind::Engine) {
        return true;
    }
    const auto opCode = static_cast<std::uintptr_t>(target->engineCommand);
    switch (view) {
    case View::Scientific:
        return opCode != IDC_PERCENT && !isProgrammerOnly(opCode);
    case View::Programmer:
        return programmerAllows(opCode, base);
    case View::Statistics:
        return statisticsAllows(opCode);
    case View::Standard:
        break;
    }
    return !isScientificOnly(opCode) && !isProgrammerOnly(opCode);
}

bool Calculator::isAvailable(std::string_view commandId) const
{
    return isAvailable(m_view, m_numberBase, commandId);
}

NumberBase Calculator::numberBase() const
{
    return m_numberBase;
}

WordSize Calculator::wordSize() const
{
    return m_wordSize;
}

int Calculator::wordBits() const
{
    switch (m_wordSize) {
    case WordSize::Dword: return 32;
    case WordSize::Word: return 16;
    case WordSize::Byte: return 8;
    case WordSize::Qword: break;
    }
    return 64;
}

std::string Calculator::bits()
{
    std::string result(bitCount, '0');
    if (m_view != View::Programmer || isError()) {
        return result;
    }
    // The engine's own binary text of the current value (two's complement within the word size)
    const std::wstring binary = m_engine->GetCurrentResultForRadix(2, programmerPrecision, false);
    std::size_t position = bitCount;
    for (auto character = binary.rbegin(); character != binary.rend() && position > 0; ++character) {
        if (*character == L'0' || *character == L'1') {
            result[--position] = static_cast<char>(*character);
        }
    }
    return result;
}

bool Calculator::isChosen(std::string_view commandId) const
{
    const auto target = findCommand(commandId);
    if (!target || target->kind != CommandKind::Engine) {
        return false;
    }
    switch (static_cast<std::uintptr_t>(target->engineCommand)) {
    case IDC_DEG: return m_angleUnit == AngleUnit::Degrees;
    case IDC_RAD: return m_angleUnit == AngleUnit::Radians;
    case IDC_GRAD: return m_angleUnit == AngleUnit::Grads;
    case IDM_HEX: return m_numberBase == NumberBase::Hexadecimal;
    case IDM_DEC: return m_numberBase == NumberBase::Decimal;
    case IDM_OCT: return m_numberBase == NumberBase::Octal;
    case IDM_BIN: return m_numberBase == NumberBase::Binary;
    case IDM_QWORD: return m_wordSize == WordSize::Qword;
    case IDM_DWORD: return m_wordSize == WordSize::Dword;
    case IDM_WORD: return m_wordSize == WordSize::Word;
    case IDM_BYTE: return m_wordSize == WordSize::Byte;
    default: return false;
    }
}

CCalcEngine* Calculator::engineOf(View view) const
{
    switch (view) {
    case View::Scientific: return m_scientificEngine.get();
    case View::Programmer: return m_programmerEngine.get();
    case View::Statistics: return m_statisticsEngine.get();
    case View::Standard: break;
    }
    return m_standardEngine.get();
}

void Calculator::runProgrammer(std::uintptr_t opCode)
{
    switch (opCode) {
    case IDM_HEX: m_numberBase = NumberBase::Hexadecimal; break;
    case IDM_DEC: m_numberBase = NumberBase::Decimal; break;
    case IDM_OCT: m_numberBase = NumberBase::Octal; break;
    case IDM_BIN: m_numberBase = NumberBase::Binary; break;
    case IDM_QWORD: m_wordSize = WordSize::Qword; break;
    case IDM_DWORD: m_wordSize = WordSize::Dword; break;
    case IDM_WORD: m_wordSize = WordSize::Word; break;
    case IDM_BYTE: m_wordSize = WordSize::Byte; break;
    default: break;
    }
    runEngine(opCode);
}

void Calculator::resetWrapperState()
{
    m_chain.clear();
    m_pendingOperator = 0;
    m_leftValue.reset();
    m_lastWasOperator = false;
    m_percentText.clear();
    m_afterEquals = false;
    m_result.reset();
}

// Windows 7 clears the display and the expression line on a view change, keeps the memory, and starts the Scientific
// view in Degrees with Inv and F-E off
void Calculator::switchView(View view)
{
    if (view == m_view) {
        return;
    }
    const CalcEngine::Rational memory = memoryValue();
    // The engine takes nothing but C while it shows an error, so it is cleared before F-E and the unit are reset
    m_scientificEngine->ProcessCommand(IDC_CLEAR);
    if (m_scientificNotation && m_view == View::Scientific) {
        m_scientificEngine->ProcessCommand(IDC_FE);
    }
    m_scientificEngine->ProcessCommand(IDC_DEG);
    m_statisticsEngine->ProcessCommand(IDC_CLEAR);
    if (m_scientificNotation && m_view == View::Statistics) {
        m_statisticsEngine->ProcessCommand(IDC_FE);
    }
    m_statisticsError.clear();
    m_inverse = false;
    m_angleUnit = AngleUnit::Degrees;
    m_scientificNotation = false;
    m_view = view;
    // The Programmer engine keeps its base and word size, as Windows 7 does
    m_engine = engineOf(view);
    m_engine->PersistedMemObject(memory);
    resetWrapperState();
    m_engine->ProcessCommand(IDC_CLEAR);
    // The number library (RatPack) keeps its radix and precision in constants shared by every engine; the engine of the
    // view must set its own again, as Windows 10 does on every view change (CalculatorManager::SetStandardMode and
    // the others), or it calculates with the previous view's (found in review: a wrong 0 or -3e-47)
    m_engine->ChangePrecision(precisionOf(view));
}

void Calculator::runScientific(std::uintptr_t opCode)
{
    if (opCode == IDC_PERCENT) {
        return;  // Windows 7 greys % in the Scientific view
    }
    switch (opCode) {
    case IDC_DEG: m_angleUnit = AngleUnit::Degrees; break;
    case IDC_RAD: m_angleUnit = AngleUnit::Radians; break;
    case IDC_GRAD: m_angleUnit = AngleUnit::Grads; break;
    default: break;
    }
    if (m_inverse && hasInverseForm(opCode)) {
        runEngine(IDC_INV);  // the engine gives the inverse form and turns its own Inv off again
        m_inverse = false;
    }
    if (opCode == IDC_CLEAR || opCode == IDC_CENTR) {
        m_inverse = false;  // C and CE turn Inv off (Windows 7)
    }
    // C (also CE while an error shows, which the engine takes as C) turns F-E off; CE keeps it (Windows 7). F-E is
    // turned off after the clear, when the engine takes commands again.
    const bool clears = opCode == IDC_CLEAR || (opCode == IDC_CENTR && isError());
    if (opCode == IDC_FE) {
        m_scientificNotation = !m_scientificNotation;
    }
    runEngine(opCode);
    if (clears) {
        if (m_scientificNotation) {
            runEngine(IDC_FE);
            m_scientificNotation = false;
        }
        resetWrapperState();
        return;
    }
    if (opCode == IDC_EQU) {
        m_afterEquals = !isError();
    } else if (opCode != IDC_STORE && opCode != IDC_MCLEAR && opCode != IDC_MPLUS && opCode != IDC_MMINUS && opCode != IDC_FE
               && opCode != IDC_DEG && opCode != IDC_RAD && opCode != IDC_GRAD) {
        m_afterEquals = false;
    }
}

// An engine command in the Statistics view. C clears the number and the wrapper's own error, and keeps the list and
// F-E (Windows 7); the engine itself keeps its F-E over C.
void Calculator::runStatistics(std::uintptr_t opCode)
{
    if (opCode == IDC_CLEAR) {
        m_statisticsError.clear();
    }
    if (opCode == IDC_FE) {
        m_scientificNotation = !m_scientificNotation;
    }
    runEngine(opCode);
}

void Calculator::runStatisticsCommand(const CommandTarget& target)
{
    switch (target.kind) {
    case CommandKind::StatisticsClearAll:
        // CAD works during an error too: it clears the number and the whole list; the memory stays (Windows 7)
        m_values.clear();
        m_selected.reset();
        m_statisticsError.clear();
        runEngine(IDC_CLEAR);
        return;
    case CommandKind::StatisticsSelect:
        // An index outside the list changes nothing
        if (target.index < m_values.size()) {
            m_selected = target.index;
        }
        return;
    case CommandKind::StatisticsSelectPrevious:
        if (m_selected && *m_selected > 0) {
            --*m_selected;
        }
        return;
    case CommandKind::StatisticsSelectNext:
        if (m_selected && *m_selected + 1 < m_values.size()) {
            ++*m_selected;
        }
        return;
    case CommandKind::StatisticsDelete:
        deleteSelectedValue();
        return;
    default:
        break;
    }
    // While an error shows, Add and the statistics are ignored (Windows 7)
    if (isError()) {
        return;
    }
    if (target.kind == CommandKind::StatisticsAdd) {
        // The number shown goes to the end of the list and is selected; the display starts again at 0
        m_values.push_back(currentValue());
        m_selected = m_values.size() - 1;
        runEngine(IDC_CLEAR);
        return;
    }
    showStatistic(target.statistic);
}

// A statistic of the list becomes the current value, as a memory recall does; the list does not change. Too few
// values give Windows 7's "Enter data to calculate".
void Calculator::showStatistic(Statistic statistic)
{
    if (!hasEnoughValues(statistic, m_values.size())) {
        m_statisticsError = enterDataText;
        return;
    }
    const std::optional<CalcEngine::Rational> result = statisticOf(statistic, m_values);
    if (!result) {
        // The number library could not compute it (no list Windows 7 accepts does this): the engine's own overflow
        m_engine->DisplayError(CALC_E_OVERFLOW);
        return;
    }
    enterValue(*result);
}

void Calculator::deleteSelectedValue()
{
    if (!m_selected || *m_selected >= m_values.size()) {
        return;
    }
    m_values.erase(m_values.begin() + static_cast<std::ptrdiff_t>(*m_selected));
    if (m_values.empty()) {
        m_selected.reset();
    } else {
        m_selected = std::min(*m_selected, m_values.size() - 1);
    }
}

std::vector<std::wstring> Calculator::statisticsValues()
{
    std::vector<std::wstring> texts;
    texts.reserve(m_values.size());
    for (const CalcEngine::Rational& value : m_values) {
        // The engine writes it as its display does: 16 digits, F-E, the regional decimal separator
        texts.push_back(m_statisticsEngine->GetStringForDisplay(value, 10));
    }
    return texts;
}

std::size_t Calculator::statisticsCount() const
{
    return m_values.size();
}

std::optional<std::size_t> Calculator::selectedValue() const
{
    return m_selected;
}

void Calculator::runEngine(std::uintptr_t opCode)
{
    m_engine->ProcessCommand(opCode);
}

void Calculator::binaryOperator(std::uintptr_t opCode)
{
    if (m_lastWasOperator && !m_chain.empty()) {
        // Another operator right after one replaces it, in the engine and on the expression line
        m_chain.back() = operatorCharacter(opCode);
    } else {
        std::wstring operand = m_percentText.empty() ? m_display->tail() : m_percentText;
        if (m_afterEquals || operand.empty()) {
            operand = display();
        }
        m_chain = (m_chain.empty() ? operand : m_chain + L' ' + operand) + L' ' + operatorCharacter(opCode);
    }
    runEngine(opCode);
    m_pendingOperator = opCode;
    m_lastWasOperator = true;
    m_afterEquals = false;
    m_percentText.clear();
    m_result.reset();
    // The left value of multiply and divide, for a Windows 7 percent: the engine shows it as its current value now
    // (unless the operator itself ended in an error, as 1/0 * does)
    if ((opCode == IDC_MUL || opCode == IDC_DIV) && !isError()) {
        m_leftValue = currentValue();
    } else {
        m_leftValue.reset();
    }
}

// Windows 7 takes y% as y * x / 100 after every operator. The engine does so after + and -, but after multiply and
// divide it takes y / 100 (Windows 10's rule), so there the engine's own formula is applied to the engine's values.
// Right after = (no operator since), x is the result: 2*3=5% is 6 * 5/100 = 0.3, and the expression line shows 0.3.
void Calculator::percent()
{
    const bool afterResult = m_pendingOperator == 0 && m_result.has_value();
    const std::optional<CalcEngine::Rational> left = afterResult ? m_result : m_leftValue;
    if (!left.has_value() || (!afterResult && m_pendingOperator != IDC_MUL && m_pendingOperator != IDC_DIV)) {
        runEngine(IDC_PERCENT);
        m_lastWasOperator = false;
        m_percentText.clear();
        m_afterEquals = false;
        return;
    }
    const CalcEngine::Rational value = currentValue();
    enterValue(value * (*left / CalcEngine::Rational(100)));
    m_lastWasOperator = false;
    m_percentText = display();
    m_afterEquals = false;
}

CalcEngine::Rational Calculator::currentValue()
{
    const CalcEngine::Rational saved = memoryValue();
    m_engine->ProcessCommand(IDC_STORE);
    const CalcEngine::Rational value = memoryValue();
    m_engine->PersistedMemObject(saved);
    return value;
}

void Calculator::enterValue(const CalcEngine::Rational& value)
{
    const CalcEngine::Rational saved = memoryValue();
    m_engine->PersistedMemObject(value);
    m_engine->ProcessCommand(IDC_RECALL);
    m_engine->PersistedMemObject(saved);
}

CalcEngine::Rational Calculator::memoryValue()
{
    const std::unique_ptr<CalcEngine::Rational> taken = m_engine->PersistedMemObject();
    const CalcEngine::Rational value = taken ? *taken : CalcEngine::Rational(0);
    m_engine->PersistedMemObject(value);
    return value;
}

}  // namespace chrono_calc
