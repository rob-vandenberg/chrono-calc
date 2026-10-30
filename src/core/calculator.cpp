#include "core/calculator.h"

#include "core/commands.h"
#include "core/enginetexts.h"

#include "CalcManager/CalculatorHistory.h"
#include "CalcManager/Command.h"
#include "CalcManager/ExpressionCommandInterface.h"
#include "Header Files/CCommand.h"
#include "Header Files/CalcEngine.h"
#include "Header Files/ICalcDisplay.h"
#include "Header Files/Rational.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/calculator.cpp 0.1.5

// --- Version history ----------------------------------------------------
// v0.1.5: Drives the engine directly (its one memory register is Windows 7's memory); Windows 7's expression line and
//         percent; the separators of the regional settings (D-017)
// v0.0.4: Initial: the engine wrapper; runs commands and keeps what the engine shows (D-015)

namespace chrono_calc {

namespace {

// Windows 10's Standard precision, which Windows 7 shows as well (16 digits: 1/3 = 0.3333333333333333)
constexpr int32_t standardPrecision = 16;
constexpr std::size_t historySize = 20;

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
        if (!tokens) {
            return;
        }
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

    // Reports the window does not use
    void SetMemorizedNumbers(const std::vector<std::wstring>& /*memorizedNumbers*/) override {}
    void SetParenthesisNumber(unsigned int /*count*/) override {}
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

private:
    std::wstring m_primary;  // empty until the engine sends its first number
    std::wstring m_tail;
    bool m_isError = false;
};

Calculator::Calculator(Separators separators)
    : m_texts(separators)
    , m_display(std::make_unique<Display>())
    , m_history(std::make_shared<CalculationManager::CalculatorHistory>(historySize))
{
    CCalcEngine::InitialOneTimeOnlySetup(m_texts);
    m_engine = std::make_unique<CCalcEngine>(false /* left to right, as the Standard view */, false, &m_texts, m_display.get(), m_history);
    // The engine never gives m_decimalSeparator a start value (CalcEngine.h:166), and SettingsChanged() only updates the
    // decimal symbol of its input when the new one differs from that value: by chance it may not, and then "0,1" is read
    // as an overflow. Reporting another separator once and then the real one makes the engine update it in every case.
    m_texts.setSeparators(Separators{.decimal = separators.decimal == L'.' ? L',' : L'.', .thousands = separators.thousands});
    m_engine->SettingsChanged();
    m_texts.setSeparators(separators);
    m_engine->SettingsChanged();
    // The memory starts at 0: a recall from a memory that was never set would read nothing
    m_engine->PersistedMemObject(CalcEngine::Rational(0));
    setStandardView();
}

// The engine points to the display, the texts and the history; it is declared last, so it is destroyed first
Calculator::~Calculator() = default;

bool Calculator::run(std::string_view commandId)
{
    const auto target = findCommand(commandId);
    if (!target) {
        return false;
    }
    if (target->kind == CommandKind::ViewStandard) {
        setStandardView();
        return true;
    }
    const auto opCode = static_cast<std::uintptr_t>(target->engineCommand);
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
    return m_display->primary();
}

std::wstring Calculator::expression() const
{
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
    return m_display->isError();
}

bool Calculator::hasMemory()
{
    return memoryValue() != CalcEngine::Rational(0);
}

void Calculator::setStandardView()
{
    m_engine->ProcessCommand(IDC_DEC);
    m_engine->ProcessCommand(IDC_CLEAR);
    m_engine->ChangePrecision(standardPrecision);
    m_engine->UpdateMaxIntDigits();
    m_chain.clear();
    m_pendingOperator = 0;
    m_leftValue.reset();
    m_lastWasOperator = false;
    m_percentText.clear();
    m_afterEquals = false;
}

void Calculator::runEngine(std::uintptr_t opCode)
{
    m_engine->ProcessCommand(opCode);
}

void Calculator::binaryOperator(std::uintptr_t opCode)
{
    if (isError()) {
        runEngine(opCode);
        return;
    }
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
    // The left value of multiply and divide, for a Windows 7 percent: the engine shows it as its current value now
    if ((opCode == IDC_MUL || opCode == IDC_DIV) && !isError()) {
        m_leftValue = currentValue();
    } else {
        m_leftValue.reset();
    }
}

// Windows 7 takes y% as y * x / 100 after every operator. The engine does so after + and -, but after multiply and
// divide it takes y / 100 (Windows 10's rule), so there the engine's own formula is applied to the engine's values.
void Calculator::percent()
{
    const std::optional<CalcEngine::Rational> left = m_leftValue;
    if (isError() || !left.has_value() || (m_pendingOperator != IDC_MUL && m_pendingOperator != IDC_DIV)) {
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
