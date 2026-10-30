#include "core/calculator.h"

#include "core/commands.h"

#include "CalcManager/CalculatorManager.h"
#include "CalcManager/ExpressionCommandInterface.h"
#include "Header Files/ICalcDisplay.h"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// --- Version ------------------------------------------------------------
// src/core/calculator.cpp 0.0.4

// --- Version history ----------------------------------------------------
// v0.0.4: Initial: the engine wrapper; runs commands and keeps what the engine shows (D-015)

namespace chrono_calc {

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

    void SetExpressionDisplay(const std::shared_ptr<std::vector<std::pair<std::wstring, int>>>& tokens,
                              const std::shared_ptr<std::vector<std::shared_ptr<IExpressionCommand>>>& /*commands*/) override
    {
        m_expression.clear();
        if (!tokens) {
            return;
        }
        // The engine's tokens carry their own spacing (" + "), so they are joined as they come
        for (const auto& token : *tokens) {
            m_expression += token.first;
        }
    }

    void SetMemorizedNumbers(const std::vector<std::wstring>& memorizedNumbers) override
    {
        m_memoryCount = memorizedNumbers.size();
    }

    // Reports the window does not use (yet)
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

    [[nodiscard]] const std::wstring& expression() const
    {
        return m_expression;
    }

    [[nodiscard]] bool isError() const
    {
        return m_isError;
    }

    [[nodiscard]] std::size_t memoryCount() const
    {
        return m_memoryCount;
    }

private:
    std::wstring m_primary;  // empty until the engine sends its first number
    std::wstring m_expression;
    bool m_isError = false;
    std::size_t m_memoryCount = 0;
};

Calculator::Calculator()
    : m_display(std::make_unique<Display>())
    , m_manager(std::make_unique<CalculationManager::CalculatorManager>(m_display.get(), &m_texts))
{
    m_manager->SetStandardMode();
}

// The manager points to the display and the texts; it is declared last, so it is destroyed first
Calculator::~Calculator() = default;

bool Calculator::run(std::string_view commandId)
{
    const auto target = findCommand(commandId);
    if (!target) {
        return false;
    }
    switch (target->kind) {
    case CommandKind::Engine:
        m_manager->SendCommand(target->engineCommand);
        break;
    case CommandKind::MemoryClear:
        m_manager->MemorizedNumberClearAll();
        break;
    case CommandKind::MemoryRecall:
        // One memory, as in Windows 7: the engine's first (and only) memory place
        if (hasMemory()) {
            m_manager->MemorizedNumberLoad(0);
        }
        break;
    case CommandKind::MemoryStore:
        // MS replaces the one memory, as in Windows 7; the engine itself would add a new memory place
        if (hasMemory() && !isError()) {
            m_manager->MemorizedNumberClearAll();
        }
        m_manager->MemorizeNumber();
        break;
    case CommandKind::MemoryAdd:
        m_manager->MemorizedNumberAdd(0);
        break;
    case CommandKind::MemorySubtract:
        m_manager->MemorizedNumberSubtract(0);
        break;
    case CommandKind::ViewStandard:
        m_manager->SetStandardMode();
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
    return m_display->expression();
}

bool Calculator::isError() const
{
    return m_display->isError();
}

bool Calculator::hasMemory() const
{
    return m_display->memoryCount() > 0;
}

std::size_t Calculator::memoryPlaces() const
{
    return m_display->memoryCount();
}

}  // namespace chrono_calc
