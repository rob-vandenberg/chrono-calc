// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// Ported from microsoft/calculator commit 79e1ac5, src/CalculatorUnitTests/CalculatorManagerTest.cpp, for Chrono Calc.

#include "engine_resource_provider.h"

#include "CalcManager/CalculatorManager.h"
#include "CalcManager/CalculatorResource.h"
#include "CalcManager/Command.h"
#include "CalcManager/ExpressionCommandInterface.h"
#include "CalcManager/NumberFormattingUtils.h"
#include "Header Files/ICalcDisplay.h"

#include <QObject>
#include <QString>
#include <QTest>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// --- Version ------------------------------------------------------------
// tests/engine/test_engine_calculatormanager.cpp 0.0.3

// --- Version history ----------------------------------------------------
// v0.0.3: Initial: Microsoft's CalculatorManagerTest ported to Qt Test, every check and expected value unchanged
//         (a failed check inside a helper ends that helper and fails the test; Qt 6.4 has no setThrowOnFail)

using CalculationManager::CalculatorManager;
using CalculationManager::Command;
using CalculationManager::IResourceProvider;
using chrono_calc::engine_tests::EngineResourceProvider;
using UnitConversionManager::NumberFormattingUtils::GetNumberDigits;
using UnitConversionManager::NumberFormattingUtils::GetNumberDigitsWholeNumberPart;
using UnitConversionManager::NumberFormattingUtils::RoundSignificantDigits;
using UnitConversionManager::NumberFormattingUtils::ToScientificNumber;
using UnitConversionManager::NumberFormattingUtils::TrimTrailingZeros;

namespace {

// QCOMPARE cannot print a std::wstring; the engine's texts are compared as QString, which it prints readably
QString text(const std::wstring& value)
{
    return QString::fromStdWString(value);
}

class CalculatorManagerDisplayTester final : public ICalcDisplay
{
public:
    CalculatorManagerDisplayTester()
    {
        Reset();
    }

    void Reset()
    {
        m_isError = false;
        m_maxDigitsCalledCount = 0;
        m_binaryOperatorReceivedCallCount = 0;
    }

    void SetPrimaryDisplay(const std::wstring& text, bool isError) override
    {
        m_primaryDisplay = text;
        m_isError = isError;
    }
    void SetIsInError(bool isError) override
    {
        m_isError = isError;
    }
    void SetExpressionDisplay(std::shared_ptr<std::vector<std::pair<std::wstring, int>>> const& tokens,
                              std::shared_ptr<std::vector<std::shared_ptr<IExpressionCommand>>> const& /*commands*/) override
    {
        m_expression.clear();

        for (const auto& currentPair : *tokens)
        {
            m_expression += currentPair.first;
        }
    }
    void SetMemorizedNumbers(const std::vector<std::wstring>& numbers) override
    {
        m_memorizedNumberStrings = numbers;
    }

    void SetParenthesisNumber(unsigned int parenthesisCount) override
    {
        m_parenDisplay = parenthesisCount;
    }

    void OnNoRightParenAdded() override
    {
        // This method is used to create a narrator announcement when a close parenthesis cannot be added because there are no open parentheses
    }

    [[nodiscard]] const std::wstring& GetPrimaryDisplay() const
    {
        return m_primaryDisplay;
    }
    [[nodiscard]] const std::wstring& GetExpression() const
    {
        return m_expression;
    }
    [[nodiscard]] const std::vector<std::wstring>& GetMemorizedNumbers() const
    {
        return m_memorizedNumberStrings;
    }
    [[nodiscard]] bool GetIsError() const
    {
        return m_isError;
    }

    void OnHistoryItemAdded(unsigned int /*addedItemIndex */) override
    {
    }

    void MaxDigitsReached() override
    {
        m_maxDigitsCalledCount++;
    }

    void InputChanged() override
    {
    }

    [[nodiscard]] int GetMaxDigitsCalledCount() const
    {
        return m_maxDigitsCalledCount;
    }

    void BinaryOperatorReceived() override
    {
        m_binaryOperatorReceivedCallCount++;
    }

    void MemoryItemChanged(unsigned int /*indexOfMemory*/) override
    {
    }

    [[nodiscard]] int GetBinaryOperatorReceivedCallCount() const
    {
        return m_binaryOperatorReceivedCallCount;
    }

private:
    std::wstring m_primaryDisplay;
    std::wstring m_expression;
    unsigned int m_parenDisplay = 0;
    bool m_isError = false;
    std::vector<std::wstring> m_memorizedNumberStrings;
    int m_maxDigitsCalledCount = 0;
    int m_binaryOperatorReceivedCallCount = 0;
};

// The original keeps the display and the manager in static members; here the test object owns one driver.
class TestDriver
{
public:
    void Initialize(std::shared_ptr<CalculatorManagerDisplayTester> displayTester, std::shared_ptr<CalculatorManager> calculatorManager)
    {
        m_displayTester = std::move(displayTester);
        m_calculatorManager = std::move(calculatorManager);
    }

    // The command list ends at the first Command::CommandNULL, as the original's arrays do
    void test(const std::wstring& expectedPrimary, const std::wstring& expectedExpression, const std::vector<Command>& testCommands,
              bool cleanup = true, bool isScientific = false)
    {
        if (cleanup)
        {
            m_calculatorManager->Reset();
        }

        if (isScientific)
        {
            m_calculatorManager->SendCommand(Command::ModeScientific);
        }

        for (const Command currentCommand : testCommands)
        {
            if (currentCommand == Command::CommandNULL)
            {
                break;
            }
            m_calculatorManager->SendCommand(currentCommand);
        }

        QCOMPARE(text(m_displayTester->GetPrimaryDisplay()), text(expectedPrimary));
        if (expectedExpression != L"N/A")
        {
            QCOMPARE(text(m_displayTester->GetExpression()), text(expectedExpression));
        }
    }

private:
    std::shared_ptr<CalculatorManagerDisplayTester> m_displayTester;
    std::shared_ptr<CalculatorManager> m_calculatorManager;
};

}  // namespace

class TestEngineCalculatormanager : public QObject
{
    Q_OBJECT

private:
    // In this order, so that the manager goes before the display and the texts it points to
    std::shared_ptr<CalculatorManagerDisplayTester> m_calculatorDisplayTester;
    std::shared_ptr<IResourceProvider> m_resourceProvider;
    std::shared_ptr<CalculatorManager> m_calculatorManager;
    TestDriver m_testDriver;

protected:
    // The list ends at the first Command::CommandNULL, as the original's arrays do
    void ExecuteCommands(const std::vector<Command>& commands)
    {
        for (const Command& command : commands)
        {
            if (command == Command::CommandNULL)
            {
                break;
            }

            m_calculatorManager->SendCommand(command);
        }
    }

    static std::vector<Command> CommandListFromStringInput(const std::wstring& input)
    {
        std::vector<Command> result{};
        for (const wchar_t ch : input)
        {
            Command asCommand = Command::CommandNULL;
            if (ch == L'.')
            {
                asCommand = Command::CommandPNT;
            }
            else if (L'0' <= ch && ch <= L'9')
            {
                const int diff = static_cast<int>(ch) - static_cast<int>(L'0');
                asCommand = static_cast<Command>(diff + static_cast<int>(Command::Command0));
            }

            if (asCommand != Command::CommandNULL)
            {
                result.push_back(asCommand);
            }
        }

        return result;
    }

    void TestMaxDigitsReachedScenario(const std::wstring& constInput)
    {
        const CalculatorManagerDisplayTester* pCalculatorDisplay = m_calculatorDisplayTester.get();

        // Make sure we're in a clean state.
        QCOMPARE(pCalculatorDisplay->GetMaxDigitsCalledCount(), 0);

        std::vector<Command> commands = CommandListFromStringInput(constInput);
        QVERIFY(!commands.empty());

        // The last element in the list should always cause MaxDigitsReached
        // Remember the command but remove from the actual input that is sent
        const Command finalInput = commands[commands.size() - 1];
        commands.pop_back();
        const std::wstring input = constInput.substr(0, constInput.length() - 1);

        m_calculatorManager->SetStandardMode();
        ExecuteCommands(commands);

        const std::wstring& expectedDisplay = input;
        std::wstring display = pCalculatorDisplay->GetPrimaryDisplay();
        QCOMPARE(text(display), text(expectedDisplay));

        m_calculatorManager->SendCommand(finalInput);

        // Verify MaxDigitsReached
        display = pCalculatorDisplay->GetPrimaryDisplay();
        QCOMPARE(text(display), text(expectedDisplay));

        // MaxDigitsReached should have been called once
        QVERIFY(0 < pCalculatorDisplay->GetMaxDigitsCalledCount());
    }

    // The last part of CalculatorManagerTestMemory, on its own to keep each function readable
    void TestMemoryLimitScenario()
    {
        const CalculatorManagerDisplayTester* pCalculatorDisplay = m_calculatorDisplayTester.get();
        std::vector<std::wstring> memorizedNumbers;

        // Memorizing 101 numbers, which exceeds the limit.
        cleanup();
        for (int i = 0; i < 101; i++)
        {
            m_calculatorManager->SendCommand(Command::Command1);
            m_calculatorManager->MemorizeNumber();
        }

        memorizedNumbers = pCalculatorDisplay->GetMemorizedNumbers();
        QCOMPARE(memorizedNumbers.size(), std::size_t{100});

        // Memorizing new number, which should show up at the top of the memory
        m_calculatorManager->SendCommand(Command::Command2);
        m_calculatorManager->MemorizeNumber();
        memorizedNumbers = pCalculatorDisplay->GetMemorizedNumbers();
        QCOMPARE(text(memorizedNumbers.at(0)), text(std::wstring(L"2")));

        // Test for trying to memorize invalid value
        m_calculatorManager->SendCommand(Command::Command2);
        m_calculatorManager->SendCommand(Command::CommandSIGN);
        m_calculatorManager->SendCommand(Command::CommandSQRT);
        m_calculatorManager->MemorizeNumber();
    }

    // The original's memory checks: the shorter of the two lists must match the start of the other
    static bool memorizedNumbersMatch(const std::vector<std::wstring>& memorizedNumbers, const std::vector<std::wstring>& expectedMemorizedNumbers)
    {
        if (memorizedNumbers.size() < expectedMemorizedNumbers.size())
        {
            return std::equal(memorizedNumbers.begin(), memorizedNumbers.end(), expectedMemorizedNumbers.begin());
        }
        return std::equal(expectedMemorizedNumbers.begin(), expectedMemorizedNumbers.end(), memorizedNumbers.begin());
    }

private slots:
    // Creates instance of CalculationManager before running tests
    void init()
    {
        // The previous manager goes first, while the display and the texts it points to still exist
        m_testDriver = TestDriver{};
        m_calculatorManager.reset();
        m_calculatorDisplayTester = std::make_shared<CalculatorManagerDisplayTester>();
        m_resourceProvider = std::make_shared<EngineResourceProvider>();
        m_calculatorManager = std::make_shared<CalculatorManager>(m_calculatorDisplayTester.get(), m_resourceProvider.get());
        m_testDriver.Initialize(m_calculatorDisplayTester, m_calculatorManager);
    }

    // Resets calculator state to start state after each test
    void cleanup()
    {
        m_calculatorManager->Reset();
        m_calculatorDisplayTester->Reset();
    }

    void calculatorManagerTestStandard()
    {
        const std::vector<Command> commands1 = { Command::Command1, Command::Command2, Command::Command3, Command::CommandPNT,
                                Command::Command4, Command::Command5, Command::Command6, Command::CommandNULL };
        m_testDriver.test(L"123.456", L"", commands1);

        const std::vector<Command> commands2 = { Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"0", L"0 + ", commands2);

        const std::vector<Command> commands3 = { Command::CommandSQRT, Command::CommandNULL };
        m_testDriver.test(L"0", L"\x221A(0)", commands3);

        const std::vector<Command> commands4 = { Command::Command2, Command::CommandADD, Command::Command3,   Command::CommandEQU,
                                Command::Command4, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"7", L"4 + 3=", commands4);

        const std::vector<Command> commands5 = { Command::Command4, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"4", L"4=", commands5);

        const std::vector<Command> commands6 = { Command::Command2,    Command::Command5,    Command::Command6,   Command::CommandSQRT,
                                Command::CommandSQRT, Command::CommandSQRT, Command::CommandNULL };
        m_testDriver.test(L"2", L"\x221A(\x221A(\x221A(256)))", commands6);

        const std::vector<Command> commands7 = { Command::Command3,   Command::CommandSUB, Command::Command6,   Command::CommandEQU,
                                Command::CommandMUL, Command::Command3,   Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"-9", L"-3 \x00D7 3=", commands7);

        const std::vector<Command> commands8 = { Command::Command9,     Command::CommandMUL, Command::Command6,   Command::CommandSUB,
                                Command::CommandCENTR, Command::Command8,   Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"46", L"54 - 8=", commands8);

        const std::vector<Command> commands9 = { Command::Command6, Command::CommandMUL, Command::Command6, Command::CommandPERCENT, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"0.36", L"6 \x00D7 0.06=", commands9);

        const std::vector<Command> commands10 = { Command::Command5, Command::Command0,       Command::CommandADD, Command::Command2,
                                 Command::Command0, Command::CommandPERCENT, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"60", L"50 + 10=", commands10);

        const std::vector<Command> commands11 = { Command::Command4, Command::CommandADD, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"8", L"4 + 4=", commands11);

        const std::vector<Command> commands12 = { Command::Command5, Command::CommandADD, Command::CommandMUL, Command::Command3, Command::CommandNULL };
        m_testDriver.test(L"3", L"5 \x00D7 ", commands12);

        const std::vector<Command> commands13 = { Command::Command1, Command::CommandEXP, Command::CommandSIGN, Command::Command9, Command::Command9,   Command::Command9,
                                 Command::Command9, Command::CommandDIV, Command::Command1,    Command::Command0, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"Overflow", L"1.e-9999 \x00F7 ", commands13);

        const std::vector<Command> commands14 = { Command::Command5, Command::Command0,       Command::CommandADD, Command::Command2,
                                 Command::Command0, Command::CommandPERCENT, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"60", L"50 + 10=", commands14);

        const std::vector<Command> commands15 = { Command::Command0, Command::CommandDIV, Command::Command0, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"Result is undefined", L"0 \x00F7 ", commands15);

        const std::vector<Command> commands16 = { Command::Command1, Command::CommandDIV, Command::Command0, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"Cannot divide by zero", L"1 \x00F7 ", commands16);

        const std::vector<Command> commands17 = { Command::Command1,     Command::Command2, Command::CommandADD, Command::Command5,
                                 Command::CommandCENTR, Command::Command2, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"14", L"14 + ", commands17);

        const std::vector<Command> commands18 = { Command::Command1, Command::Command0, Command::Command0, Command::CommandSIGN, Command::CommandREC, Command::CommandNULL };
        m_testDriver.test(L"-0.01", L"1/(-100)", commands18);

        const std::vector<Command> commands19 = { Command::Command1, Command::Command2, Command::Command3, Command::CommandBACK, Command::CommandBACK, Command::CommandNULL };
        m_testDriver.test(L"1", L"", commands19);

        const std::vector<Command> commands20 = { Command::Command1,    Command::Command2,    Command::Command3,   Command::CommandBACK,
                                 Command::CommandBACK, Command::CommandBACK, Command::CommandNULL };
        m_testDriver.test(L"0", L"", commands20);

        const std::vector<Command> commands21 = { Command::Command4, Command::CommandSQRT, Command::CommandSUB, Command::Command2, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"0", L"0 + ", commands21);

        const std::vector<Command> commands22 = { Command::Command1,   Command::Command0, Command::Command2, Command::Command4,   Command::CommandSQRT,
                                 Command::CommandSUB, Command::Command3, Command::Command2, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"0", L"0 + ", commands22);

        const std::vector<Command> commands23 = { Command::Command2, Command::CommandPNT, Command::Command2, Command::Command5,   Command::CommandSQRT, Command::CommandSUB,
                                 Command::Command1, Command::CommandPNT, Command::Command5, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"0", L"\x221A(2.25) - 1.5=", commands23);
    }

    void calculatorManagerTestScientific()
    {
        const std::vector<Command> commands1 = { Command::Command1, Command::Command2, Command::Command3, Command::CommandPNT,
                                Command::Command4, Command::Command5, Command::Command6, Command::CommandNULL };
        m_testDriver.test(L"123.456", L"", commands1, true, true);

        const std::vector<Command> commands2 = { Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"0", L"0 + ", commands2, true, true);

        const std::vector<Command> commands3 = { Command::CommandSQRT, Command::CommandNULL };
        m_testDriver.test(L"0", L"\x221A(0)", commands3, true, true);

        const std::vector<Command> commands4 = { Command::Command1, Command::CommandADD, Command::Command0,   Command::CommandMUL,
                                Command::Command2, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"1", L"1 + 0 \x00D7 2=", commands4, true, true);

        const std::vector<Command> commands5 = { Command::Command4, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"4", L"4=", commands5, true, true);

        const std::vector<Command> commands6 = { Command::Command2,    Command::Command5,    Command::Command6,   Command::CommandSQRT,
                                Command::CommandSQRT, Command::CommandSQRT, Command::CommandNULL };
        m_testDriver.test(L"2", L"\x221A(\x221A(\x221A(256)))", commands6, true, true);

        const std::vector<Command> commands7 = { Command::Command3,   Command::CommandSUB, Command::Command6,   Command::CommandEQU,
                                Command::CommandMUL, Command::Command3,   Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"-9", L"-3 \x00D7 3 + ", commands7, true, true);

        const std::vector<Command> commands8 = { Command::Command9, Command::CommandMUL, Command::Command6, Command::CommandSUB, Command::CommandCENTR,
                                Command::Command8, Command::CommandMUL, Command::Command2, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"38", L"9 \x00D7 6 - 8 \x00D7 2 + ", commands8, true, true);

        const std::vector<Command> commands9 = { Command::Command6, Command::CommandMUL, Command::Command6, Command::CommandSIGN, Command::CommandSQRT, Command::CommandNULL };
        m_testDriver.test(L"Invalid input", L"6 \x00D7 \x221A(-6)", commands9, true, true);

        const std::vector<Command> commands10 = { Command::Command5, Command::Command0,   Command::CommandADD, Command::Command2,
                                 Command::Command0, Command::CommandREC, Command::CommandSUB, Command::CommandNULL };
        m_testDriver.test(L"50.05", L"50 + 1/(20) - ", commands10, true, true);

        const std::vector<Command> commands11 = { Command::Command4, Command::CommandADD, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"8", L"4 + 4=", commands11, true, true);

        const std::vector<Command> commands12 = { Command::Command5, Command::CommandADD, Command::CommandMUL, Command::Command3, Command::CommandNULL };
        m_testDriver.test(L"3", L"5 \x00D7 ", commands12, true, true);

        const std::vector<Command> commands13 = { Command::Command1, Command::CommandEXP, Command::CommandSIGN, Command::Command9, Command::Command9,   Command::Command9,
                                 Command::Command9, Command::CommandDIV, Command::Command1,    Command::Command0, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"Overflow", L"1.e-9999 \x00F7 ", commands13, true, true);

        const std::vector<Command> commands14 = { Command::Command5, Command::Command0,       Command::CommandADD, Command::Command2,
                                 Command::Command0, Command::CommandPERCENT, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"60", L"50 + 10=", commands14, true, true);

        const std::vector<Command> commands15 = { Command::Command0, Command::CommandDIV, Command::Command0, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"Result is undefined", L"0 \x00F7 ", commands15, true, true);

        const std::vector<Command> commands16 = { Command::Command1, Command::CommandDIV, Command::Command0, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"Cannot divide by zero", L"1 \x00F7 ", commands16, true, true);

        const std::vector<Command> commands17 = { Command::Command1,     Command::Command2, Command::CommandADD, Command::Command5,
                                 Command::CommandCENTR, Command::Command2, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"14", L"12 + 2 + ", commands17, true, true);

        const std::vector<Command> commands18 = { Command::Command1, Command::Command0, Command::Command0, Command::CommandSIGN, Command::CommandREC, Command::CommandNULL };
        m_testDriver.test(L"-0.01", L"1/(-100)", commands18, true, true);

        const std::vector<Command> commands19 = { Command::Command1, Command::Command2, Command::Command3, Command::CommandBACK, Command::CommandBACK, Command::CommandNULL };
        m_testDriver.test(L"1", L"", commands19, true, true);

        const std::vector<Command> commands20 = { Command::Command1,    Command::Command2,    Command::Command3,   Command::CommandBACK,
                                 Command::CommandBACK, Command::CommandBACK, Command::CommandNULL };
        m_testDriver.test(L"0", L"", commands20, true, true);

        const std::vector<Command> commands21 = { Command::Command4, Command::CommandSQRT, Command::CommandSUB, Command::Command2, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"0", L"\x221A(4) - 2 + ", commands21, true, true);

        const std::vector<Command> commands22 = { Command::Command0, Command::CommandSQRT, Command::CommandNULL };
        m_testDriver.test(L"0", L"\x221A(0)", commands22, true, true);

        const std::vector<Command> commands23 = { Command::Command1,   Command::Command0, Command::Command2, Command::Command4,   Command::CommandSQRT,
                                 Command::CommandSUB, Command::Command3, Command::Command2, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"0", L"\x221A(1024) - 32 + ", commands23, true, true);

        const std::vector<Command> commands24 = { Command::Command2,    Command::Command5,    Command::Command7,   Command::CommandSQRT,
                                 Command::CommandSQRT, Command::CommandSQRT, Command::CommandNULL };
        m_testDriver.test(L"2.0009748976330773374220277351385", L"\x221A(\x221A(\x221A(257)))", commands24, true, true);

        const std::vector<Command> commands25 = { Command::Command2, Command::CommandPNT, Command::Command2, Command::Command5,   Command::CommandSQRT, Command::CommandSUB,
                                 Command::Command1, Command::CommandPNT, Command::Command5, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"0", L"\x221A(2.25) - 1.5=", commands25, true, true);

        const std::vector<Command> commands26 = { Command::CommandOPENP, Command::Command2,   Command::CommandPNT, Command::Command2,   Command::Command5,
                                 Command::CommandSQRT,  Command::CommandDIV, Command::Command1,   Command::CommandPNT, Command::Command5,
                                 Command::CommandCLOSEP, Command::CommandLOG,  Command::CommandNULL };
        m_testDriver.test(L"0", L"log(\x221A(2.25) \x00F7 1.5)", commands26, true, true);
    }

    // Scientific functions from the scientific calculator
    void calculatorManagerTestScientific2()
    {
        const std::vector<Command> commands1 = { Command::Command1, Command::Command2, Command::CommandSQR, Command::CommandNULL };
        m_testDriver.test(L"144", L"sqr(12)", commands1, true, true);

        const std::vector<Command> commands2 = { Command::Command5, Command::CommandFAC, Command::CommandNULL };
        m_testDriver.test(L"120", L"fact(5)", commands2, true, true);

        const std::vector<Command> commands3 = { Command::Command5, Command::CommandPWR, Command::Command2, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"25", L"5 ^ 2 + ", commands3, true, true);

        const std::vector<Command> commands4 = { Command::Command8, Command::CommandROOT, Command::Command3, Command::CommandMUL, Command::CommandNULL };
        m_testDriver.test(L"2", L"8 yroot 3 \x00D7 ", commands4, true, true);

        const std::vector<Command> commands5 = { Command::Command8, Command::CommandCUB, Command::CommandNULL };
        m_testDriver.test(L"512", L"cube(8)", commands5, true, true);

        const std::vector<Command> commands6 = { Command::Command8, Command::CommandCUB, Command::CommandCUBEROOT, Command::CommandNULL };
        m_testDriver.test(L"8", L"cuberoot(cube(8))", commands6, true, true);

        const std::vector<Command> commands7 = { Command::Command1, Command::Command0, Command::CommandLOG, Command::CommandNULL };
        m_testDriver.test(L"1", L"log(10)", commands7, true, true);

        const std::vector<Command> commands8 = { Command::Command5, Command::CommandPOW10, Command::CommandNULL };
        m_testDriver.test(L"100,000", L"10^(5)", commands8, true, true);

        const std::vector<Command> commands9 = { Command::Command1, Command::Command0, Command::CommandLN, Command::CommandNULL };
        m_testDriver.test(L"2.3025850929940456840179914546844", L"ln(10)", commands9, true, true);

        const std::vector<Command> commands10 = { Command::Command1, Command::CommandSIN, Command::CommandNULL };
        m_testDriver.test(L"0.01745240643728351281941897851632", L"sin\x2080(1)", commands10, true, true);

        const std::vector<Command> commands11 = { Command::Command1, Command::CommandCOS, Command::CommandNULL };
        m_testDriver.test(L"0.99984769515639123915701155881391", L"cos\x2080(1)", commands11, true, true);

        const std::vector<Command> commands12 = { Command::Command1, Command::CommandTAN, Command::CommandNULL };
        m_testDriver.test(L"0.01745506492821758576512889521973", L"tan\x2080(1)", commands12, true, true);

        const std::vector<Command> commands13 = { Command::Command1, Command::CommandASIN, Command::CommandNULL };
        m_testDriver.test(L"90", L"sin\x2080\x207B\x00B9(1)", commands13, true, true);

        const std::vector<Command> commands14 = { Command::Command1, Command::CommandACOS, Command::CommandNULL };
        m_testDriver.test(L"0", L"cos\x2080\x207B\x00B9(1)", commands14, true, true);

        const std::vector<Command> commands15 = { Command::Command1, Command::CommandATAN, Command::CommandNULL };
        m_testDriver.test(L"45", L"tan\x2080\x207B\x00B9(1)", commands15, true, true);

        const std::vector<Command> commands16 = { Command::Command2, Command::CommandPOWE, Command::CommandNULL };
        m_testDriver.test(L"7.389056098930650227230427460575", L"e^(2)", commands16, true, true);

        const std::vector<Command> commands17 = { Command::Command5, Command::CommandPWR, Command::Command0, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"1", L"5 ^ 0 + ", commands17, true, true);

        const std::vector<Command> commands18 = { Command::Command0, Command::CommandPWR, Command::Command0, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"1", L"0 ^ 0 + ", commands18, true, true);

        const std::vector<Command> commands19 = { Command::Command2, Command::Command7,   Command::CommandSIGN, Command::CommandROOT,
                                 Command::Command3, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"-3", L"-27 yroot 3 + ", commands19, true, true);

        const std::vector<Command> commands20 = { Command::Command8,   Command::CommandPWR, Command::CommandOPENP,  Command::Command2,
                                 Command::CommandDIV, Command::Command3,   Command::CommandCLOSEP, Command::CommandSUB,
                                 Command::Command4,   Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"0", L"8 ^ (2 \x00F7 3) - 4 + ", commands20, true, true);

        const std::vector<Command> commands21 = { Command::Command4,   Command::CommandPWR, Command::CommandOPENP,  Command::Command3,
                                 Command::CommandDIV, Command::Command2,   Command::CommandCLOSEP, Command::CommandSUB,
                                 Command::Command8,   Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"0", L"4 ^ (3 \x00F7 2) - 8 + ", commands21, true, true);

        const std::vector<Command> commands22 = { Command::Command1, Command::Command0, Command::CommandPWR, Command::Command1, Command::CommandPNT, Command::Command2,
                                 Command::Command3, Command::Command4, Command::Command5,   Command::Command6, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"17.161687912241792074207286679393", L"10 ^ 1.23456 + ", commands22, true, true);

        const std::vector<Command> commands23 = { Command::Command1, Command::CommandSEC, Command::CommandNULL };
        m_testDriver.test(L"1.0001523280439076654284264342126", L"sec\x2080(1)", commands23, true, true);

        const std::vector<Command> commands24 = { Command::Command1, Command::CommandCSC, Command::CommandNULL };
        m_testDriver.test(L"57.298688498550183476612683735174", L"csc\x2080(1)", commands24, true, true);

        const std::vector<Command> commands25 = { Command::Command1, Command::CommandCOT, Command::CommandNULL };
        m_testDriver.test(L"57.289961630759424687278147537113", L"cot\x2080(1)", commands25, true, true);

        const std::vector<Command> commands26 = { Command::Command1, Command::CommandASEC, Command::CommandNULL };
        m_testDriver.test(L"0", L"sec\x2080\x207B\x00B9(1)", commands26, true, true);

        const std::vector<Command> commands27 = { Command::Command1, Command::CommandACSC, Command::CommandNULL };
        m_testDriver.test(L"90", L"csc\x2080\x207B\x00B9(1)", commands27, true, true);

        const std::vector<Command> commands28 = { Command::Command1, Command::CommandACOT, Command::CommandNULL };
        m_testDriver.test(L"45", L"cot\x2080\x207B\x00B9(1)", commands28, true, true);

        const std::vector<Command> commands29 = { Command::Command1, Command::CommandSECH, Command::CommandNULL };
        m_testDriver.test(L"0.64805427366388539957497735322615", L"sech(1)", commands29, true, true);

        const std::vector<Command> commands30 = { Command::Command1, Command::CommandCSCH, Command::CommandNULL };
        m_testDriver.test(L"0.85091812823932154513384276328718", L"csch(1)", commands30, true, true);

        const std::vector<Command> commands31 = { Command::Command1, Command::CommandCOTH, Command::CommandNULL };
        m_testDriver.test(L"1.3130352854993313036361612469308", L"coth(1)", commands31, true, true);

        const std::vector<Command> commands32 = { Command::Command1, Command::CommandASECH, Command::CommandNULL };
        m_testDriver.test(L"0", L"sech\x207B\x00B9(1)", commands32, true, true);

        const std::vector<Command> commands33 = { Command::Command1, Command::CommandACSCH, Command::CommandNULL };
        m_testDriver.test(L"0.88137358701954302523260932497979", L"csch\x207B\x00B9(1)", commands33, true, true);

        const std::vector<Command> commands34 = { Command::Command2, Command::CommandACOTH, Command::CommandNULL };
        m_testDriver.test(L"0.54930614433405484569762261846126", L"coth\x207B\x00B9(2)", commands34, true, true);

        const std::vector<Command> commands35 = { Command::Command8, Command::CommandPOW2, Command::CommandNULL };
        m_testDriver.test(L"256", L"2^(8)", commands35, true, true);

        const std::vector<Command> commands36 = { Command::CommandRand, Command::CommandCeil, Command::CommandNULL };
        m_testDriver.test(L"1", L"N/A", commands36, true, true);

        const std::vector<Command> commands37 = { Command::CommandRand, Command::CommandFloor, Command::CommandNULL };
        m_testDriver.test(L"0", L"N/A", commands37, true, true);

        const std::vector<Command> commands38 = { Command::CommandRand, Command::CommandSIGN, Command::CommandCeil, Command::CommandNULL };
        m_testDriver.test(L"0", L"N/A", commands38, true, true);

        const std::vector<Command> commands39 = { Command::CommandRand, Command::CommandSIGN, Command::CommandFloor, Command::CommandNULL };
        m_testDriver.test(L"-1", L"N/A", commands39, true, true);

        const std::vector<Command> commands40 = { Command::Command3, Command::CommandPNT, Command::Command8, Command::CommandFloor, Command::CommandNULL };
        m_testDriver.test(L"3", L"floor(3.8)", commands40, true, true);

        const std::vector<Command> commands41 = { Command::Command3, Command::CommandPNT, Command::Command8, Command::CommandCeil, Command::CommandNULL };
        m_testDriver.test(L"4", L"ceil(3.8)", commands41, true, true);

        const std::vector<Command> commands42 = { Command::Command5, Command::CommandLogBaseY, Command::Command3, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"1.4649735207179271671970404076786", L"5 log base 3 + ", commands42, true, true);
    }

    void calculatorManagerTestScientificParenthesis()
    {
        const std::vector<Command> commands1 = { Command::Command1, Command::CommandADD,    Command::CommandOPENP, Command::CommandADD,
                                Command::Command3, Command::CommandCLOSEP, Command::CommandNULL };
        m_testDriver.test(L"3", L"1 + (0 + 3)", commands1, true, true);

        const std::vector<Command> commands2 = {
            Command::CommandOPENP, Command::CommandOPENP, Command::Command1, Command::Command2, Command::CommandCLOSEP, Command::CommandNULL
        };
        m_testDriver.test(L"12", L"((12)", commands2, true, true);

        const std::vector<Command> commands3 = { Command::Command1,      Command::Command2,     Command::CommandCLOSEP,
                                Command::CommandCLOSEP, Command::CommandOPENP, Command::CommandNULL };
        m_testDriver.test(L"12", L"12 \x00D7 (", commands3, true, true);

        const std::vector<Command> commands4 = {
            Command::Command2, Command::CommandOPENP, Command::Command2, Command::CommandCLOSEP, Command::CommandADD, Command::CommandNULL
        };
        m_testDriver.test(L"4", L"2 \x00D7 (2) + ", commands4, true, true);

        const std::vector<Command> commands5 = { Command::Command2,   Command::CommandOPENP, Command::Command2,   Command::CommandCLOSEP,
                                Command::CommandADD, Command::CommandEQU,   Command::CommandNULL };
        m_testDriver.test(L"8", L"2 \x00D7 (2) + 4=", commands5, true, true);

        const std::vector<Command> commands6 = { Command::CommandOPENP, Command::Command8, Command::CommandCLOSEP, Command::Command2, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"16", L"(8) \x00D7 2=", commands6, true, true);

        const std::vector<Command> commands7 = { Command::CommandOPENP,  Command::Command7, Command::CommandMUL, Command::Command2,
                                Command::CommandCLOSEP, Command::Command2, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"28", L"(7 \x00D7 2) \x00D7 2=", commands7, true, true);

        const std::vector<Command> commands8 = { Command::CommandOPENP,  Command::Command7, Command::CommandMUL, Command::Command2,
                                Command::CommandCLOSEP, Command::Command2, Command::CommandEQU,
                                Command::CommandOPENP, Command::Command1, Command::Command4, Command::CommandCLOSEP, Command::Command2, Command::CommandEQU, Command::CommandNULL};
        m_testDriver.test(L"28", L"(14) \x00D7 2=", commands8, true, true);

        const std::vector<Command> commands9 = { Command::CommandOPENP, Command::Command8, Command::CommandCLOSEP,
                                Command::Command0, Command::CommandPNT, Command::Command5, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"4", L"(8) \x00D7 0.5=", commands9, true, true);

        const std::vector<Command> commands10 = { Command::CommandOPENP, Command::Command8, Command::CommandCLOSEP,
                                 Command::CommandPNT, Command::Command5, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"4", L"(8) \x00D7 0.5=", commands10, true, true);
    }

    void calculatorManagerTestScientificError()
    {
        const std::vector<Command> commands1 = { Command::Command1, Command::CommandDIV, Command::Command0, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"Cannot divide by zero", L"1 \x00F7 ", commands1, true, true);
        QVERIFY(m_calculatorDisplayTester->GetIsError());

        const std::vector<Command> commands2 = { Command::Command2, Command::CommandSIGN, Command::CommandLOG, Command::CommandNULL };
        m_testDriver.test(L"Invalid input", L"log(-2)", commands2, true, true);
        QVERIFY(m_calculatorDisplayTester->GetIsError());

        const std::vector<Command> commands3 = { Command::Command0, Command::CommandDIV, Command::Command0, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"Result is undefined", L"0 \x00F7 ", commands3, true, true);
        QVERIFY(m_calculatorDisplayTester->GetIsError());

        // Do the same tests for the basic calculator
        m_testDriver.test(L"Cannot divide by zero", L"1 \x00F7 ", commands1);
        QVERIFY(m_calculatorDisplayTester->GetIsError());
        m_testDriver.test(L"Invalid input", L"log(-2)", commands2);
        QVERIFY(m_calculatorDisplayTester->GetIsError());
        m_testDriver.test(L"Result is undefined", L"0 \x00F7 ", commands3);
        QVERIFY(m_calculatorDisplayTester->GetIsError());
    }

    // Radians and Grads Test
    void calculatorManagerTestScientificModeChange()
    {
        const std::vector<Command> commands1 = { Command::CommandRAD, Command::CommandPI, Command::CommandSIN, Command::CommandNULL };
        m_testDriver.test(L"0", L"N/A", commands1, true, true);

        const std::vector<Command> commands2 = { Command::CommandRAD, Command::CommandPI, Command::CommandCOS, Command::CommandNULL };
        m_testDriver.test(L"-1", L"N/A", commands2, true, true);

        const std::vector<Command> commands3 = { Command::CommandRAD, Command::CommandPI, Command::CommandTAN, Command::CommandNULL };
        m_testDriver.test(L"0", L"N/A", commands3, true, true);

        const std::vector<Command> commands4 = { Command::CommandGRAD, Command::Command4, Command::Command0, Command::Command0, Command::CommandSIN, Command::CommandNULL };
        m_testDriver.test(L"0", L"N/A", commands4, true, true);

        const std::vector<Command> commands5 = { Command::CommandGRAD, Command::Command4, Command::Command0, Command::Command0, Command::CommandCOS, Command::CommandNULL };
        m_testDriver.test(L"1", L"N/A", commands5, true, true);

        const std::vector<Command> commands6 = { Command::CommandGRAD, Command::Command4, Command::Command0, Command::Command0, Command::CommandTAN, Command::CommandNULL };
        m_testDriver.test(L"0", L"N/A", commands6, true, true);
    }

    void calculatorManagerTestProgrammer()
    {
        const std::vector<Command> commands1 = { Command::ModeProgrammer, Command::Command5, Command::Command3,   Command::CommandNand,
                                Command::Command8,       Command::Command3, Command::CommandAnd, Command::CommandNULL };
        m_testDriver.test(L"-18", L"53 NAND 83 AND ", commands1, true, false);

        const std::vector<Command> commands2 = { Command::ModeProgrammer, Command::Command5, Command::Command3,   Command::CommandNor,
                                Command::Command8,       Command::Command3, Command::CommandAnd, Command::CommandNULL };
        m_testDriver.test(L"-120", L"53 NOR 83 AND ", commands2, true, false);

        const std::vector<Command> commands3 = {
            Command::ModeProgrammer, Command::Command5, Command::CommandLSHF, Command::Command1, Command::CommandAnd, Command::CommandNULL
        };
        m_testDriver.test(L"10", L"5 Lsh 1 AND ", commands3, true, false);

        const std::vector<Command> commands5 = {
            Command::ModeProgrammer, Command::Command5, Command::CommandRSHFL, Command::Command1, Command::CommandAnd, Command::CommandNULL
        };
        m_testDriver.test(L"2", L"5 Rsh 1 AND ", commands5, true, false);

        const std::vector<Command> commands6 = { Command::ModeProgrammer, Command::CommandBINPOS63, Command::CommandRSHF, Command::Command5,
                                Command::Command6,       Command::CommandAnd,      Command::CommandNULL };
        m_testDriver.test(L"-128", L"-9223372036854775808 Rsh 56 AND ", commands6, true, false);

        const std::vector<Command> commands7 = { Command::ModeProgrammer, Command::Command1, Command::CommandROL, Command::CommandNULL };
        m_testDriver.test(L"2", L"RoL(1)", commands7, true, false);

        const std::vector<Command> commands8 = { Command::ModeProgrammer, Command::Command1, Command::CommandROR, Command::CommandNULL };
        m_testDriver.test(L"-9,223,372,036,854,775,808", L"RoR(1)", commands8, true, false);

        const std::vector<Command> commands9 = { Command::ModeProgrammer, Command::Command1, Command::CommandRORC, Command::CommandNULL };
        m_testDriver.test(L"0", L"RoR(1)", commands9, true, false);

        const std::vector<Command> commands10 = { Command::ModeProgrammer, Command::Command1, Command::CommandRORC, Command::CommandRORC, Command::CommandNULL };
        m_testDriver.test(L"-9,223,372,036,854,775,808", L"RoR(RoR(1))", commands10, true, false);

        const std::vector<Command> commands11 = { Command::ModeProgrammer, Command::CommandDec, Command::Command4, Command::Command2, Command::Command9,   Command::Command4,
                                Command::Command9,       Command::Command6,   Command::Command7, Command::Command2, Command::Command9,   Command::Command6,
                                Command::CommandDIV,     Command::Command2,   Command::Command5, Command::Command5, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"16,843,009", L"4294967296 \x00F7 255=", commands11, true, false);

        const std::vector<Command> commands12 = {
            Command::ModeProgrammer, Command::CommandDec, Command::Command4, Command::Command2, Command::Command9,   Command::Command4,
            Command::Command9,       Command::Command6,   Command::Command7, Command::Command3, Command::Command0,   Command::Command3,
            Command::CommandDIV,     Command::Command2,   Command::Command5, Command::Command5, Command::CommandEQU, Command::CommandNULL
        };
        m_testDriver.test(L"16,843,009", L"4294967303 \x00F7 255=", commands12, true, false);

        const std::vector<Command> commands13 = {
            Command::ModeProgrammer, Command::CommandDec, Command::Command1, Command::Command0, Command::Command0, Command::Command0,
            Command::Command0, Command::Command0, Command::Command0, Command::Command0, Command::Command0, Command::Command0, Command::CommandDIV,
            Command::Command6, Command::Command4, Command::Command4, Command::Command8, Command::Command7, Command::CommandEQU, Command::CommandNULL
        };
        m_testDriver.test(L"15,507", L"1000000000 \x00F7 64487=", commands13, true, false);

        const std::vector<Command> commands14 = { Command::ModeProgrammer, Command::CommandDec, Command::Command1,   Command::Command0,   Command::Command0,
                                 Command::Command0,       Command::Command0,   Command::Command0,   Command::Command0,   Command::Command0,
                                 Command::Command0,       Command::Command0,   Command::CommandDIV, Command::Command6,   Command::Command4,
                                 Command::Command4,       Command::Command8,   Command::Command8,   Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"15,506", L"1000000000 \x00F7 64488=", commands14, true, false);
    }

    void calculatorManagerTestModeChange()
    {
        const std::vector<Command> commands1 = { Command::Command1, Command::Command2, Command::Command3, Command::CommandNULL };
        m_testDriver.test(L"123", L"", commands1, true, false);

        const std::vector<Command> commands2 = { Command::ModeScientific, Command::CommandNULL };
        m_testDriver.test(L"0", L"", commands2, true, false);

        const std::vector<Command> commands3 = { Command::Command1, Command::Command2, Command::Command3, Command::CommandNULL };
        m_testDriver.test(L"123", L"", commands3, true, false);

        const std::vector<Command> commands4 = { Command::ModeProgrammer, Command::CommandNULL };
        m_testDriver.test(L"0", L"", commands4, true, false);

        const std::vector<Command> commands5 = { Command::Command1, Command::Command2, Command::Command3, Command::CommandNULL };
        m_testDriver.test(L"123", L"", commands5, true, false);

        const std::vector<Command> commands6 = { Command::ModeScientific, Command::CommandNULL };
        m_testDriver.test(L"0", L"", commands6, true, false);

        const std::vector<Command> commands7 = { Command::Command6, Command::Command7, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"67", L"67 + ", commands7, true, false);

        const std::vector<Command> commands8 = { Command::ModeBasic, Command::CommandNULL };
        m_testDriver.test(L"0", L"", commands8, true, false);
    }

    void calculatorManagerTestMemory()
    {
        const std::vector<Command> scientificCalculatorTest52 = { Command::Command1, Command::CommandSTORE, Command::CommandNULL };
        const std::wstring expectedPrimaryDisplayTestScientific52(L"1");

        const std::vector<Command> scientificCalculatorTest53 = { Command::Command1, Command::CommandNULL };

        const CalculatorManagerDisplayTester* pCalculatorDisplay = m_calculatorDisplayTester.get();
        std::wstring resultPrimary;
        std::wstring resultExpression;

        cleanup();
        ExecuteCommands(scientificCalculatorTest52);
        resultPrimary = pCalculatorDisplay->GetPrimaryDisplay();
        resultExpression = pCalculatorDisplay->GetExpression();
        QCOMPARE(text(resultPrimary), text(expectedPrimaryDisplayTestScientific52));

        cleanup();
        ExecuteCommands(scientificCalculatorTest53);
        m_calculatorManager->MemorizeNumber();
        m_calculatorManager->SendCommand(Command::CommandCLEAR);
        m_calculatorManager->MemorizedNumberLoad(0);
        resultPrimary = pCalculatorDisplay->GetPrimaryDisplay();
        resultExpression = pCalculatorDisplay->GetExpression();
        QCOMPARE(text(resultPrimary), text(expectedPrimaryDisplayTestScientific52));

        cleanup();
        m_calculatorManager->SendCommand(Command::Command1);
        m_calculatorManager->MemorizeNumber();
        m_calculatorManager->SendCommand(Command::CommandCLEAR);
        m_calculatorManager->SendCommand(Command::Command2);
        m_calculatorManager->MemorizeNumber();
        m_calculatorManager->SendCommand(Command::CommandCLEAR);
        m_calculatorManager->MemorizedNumberLoad(1);
        resultPrimary = pCalculatorDisplay->GetPrimaryDisplay();
        QCOMPARE(text(resultPrimary), text(std::wstring(L"1")));

        m_calculatorManager->MemorizedNumberLoad(0);
        resultPrimary = pCalculatorDisplay->GetPrimaryDisplay();
        QCOMPARE(text(resultPrimary), text(std::wstring(L"2")));

        cleanup();
        m_calculatorManager->SendCommand(Command::Command1);
        m_calculatorManager->SendCommand(Command::CommandSIGN);
        m_calculatorManager->MemorizeNumber();
        m_calculatorManager->SendCommand(Command::CommandADD);
        m_calculatorManager->SendCommand(Command::Command2);
        m_calculatorManager->SendCommand(Command::CommandEQU);
        m_calculatorManager->MemorizeNumber();
        m_calculatorManager->SendCommand(Command::CommandMUL);
        m_calculatorManager->SendCommand(Command::Command2);
        m_calculatorManager->MemorizeNumber();

        std::vector<std::wstring> memorizedNumbers = pCalculatorDisplay->GetMemorizedNumbers();

        std::vector<std::wstring> expectedMemorizedNumbers;
        expectedMemorizedNumbers.emplace_back(L"2");
        expectedMemorizedNumbers.emplace_back(L"1");
        expectedMemorizedNumbers.emplace_back(L"-1");

        bool isEqual = memorizedNumbersMatch(memorizedNumbers, expectedMemorizedNumbers);
        QVERIFY(isEqual);

        m_calculatorManager->SendCommand(Command::CommandCLEAR);
        m_calculatorManager->SendCommand(Command::Command2);
        m_calculatorManager->MemorizedNumberAdd(0);
        m_calculatorManager->MemorizedNumberAdd(1);
        m_calculatorManager->MemorizedNumberAdd(2);

        memorizedNumbers = pCalculatorDisplay->GetMemorizedNumbers();

        expectedMemorizedNumbers.clear();
        expectedMemorizedNumbers.emplace_back(L"4");
        expectedMemorizedNumbers.emplace_back(L"3");
        expectedMemorizedNumbers.emplace_back(L"1");

        isEqual = memorizedNumbersMatch(memorizedNumbers, expectedMemorizedNumbers);
        QVERIFY(isEqual);

        m_calculatorManager->SendCommand(Command::CommandCLEAR);
        m_calculatorManager->SendCommand(Command::Command1);
        m_calculatorManager->SendCommand(Command::CommandPNT);
        m_calculatorManager->SendCommand(Command::Command5);

        m_calculatorManager->MemorizedNumberSubtract(0);
        m_calculatorManager->MemorizedNumberSubtract(1);
        m_calculatorManager->MemorizedNumberSubtract(2);

        memorizedNumbers = pCalculatorDisplay->GetMemorizedNumbers();

        expectedMemorizedNumbers.clear();
        expectedMemorizedNumbers.emplace_back(L"2.5");
        expectedMemorizedNumbers.emplace_back(L"1.5");
        expectedMemorizedNumbers.emplace_back(L"-0.5");

        isEqual = memorizedNumbersMatch(memorizedNumbers, expectedMemorizedNumbers);
        QVERIFY(isEqual);

        TestMemoryLimitScenario();
    }

    // Send 12345678910111213 and verify MaxDigitsReached
    void calculatorManagerTestMaxDigitsReached()
    {
        TestMaxDigitsReachedScenario(L"1,234,567,891,011,1213");
    }

    void calculatorManagerTestMaxDigitsReached_LeadingDecimal()
    {
        TestMaxDigitsReachedScenario(L"0.12345678910111213");
    }

    void calculatorManagerTestMaxDigitsReached_TrailingDecimal()
    {
        TestMaxDigitsReachedScenario(L"123,456,789,101,112.13");
    }

    static void unitConversionManagerNumberFormattingUtils_TrimTrailingZeros()
    {
        std::wstring number = L"2.1032100000000";
        TrimTrailingZeros(number);
        QCOMPARE(text(number), text(L"2.10321"));
        number = L"-122.123200";
        TrimTrailingZeros(number);
        QCOMPARE(text(number), text(L"-122.1232"));
        number = L"0.0001200";
        TrimTrailingZeros(number);
        QCOMPARE(text(number), text(L"0.00012"));
        number = L"12.000";
        TrimTrailingZeros(number);
        QCOMPARE(text(number), text(L"12"));
        number = L"-12.00000";
        TrimTrailingZeros(number);
        QCOMPARE(text(number), text(L"-12"));
        number = L"0.000";
        TrimTrailingZeros(number);
        QCOMPARE(text(number), text(L"0"));
        number = L"322423";
        TrimTrailingZeros(number);
        QCOMPARE(text(number), text(L"322423"));
    }

    static void unitConversionManagerNumberFormattingUtils_GetNumberDigits()
    {
        std::wstring number = L"2.10321";
        unsigned int digitsCount = GetNumberDigits(number);
        QCOMPARE(digitsCount, 6U);
        number = L"-122.1232";
        digitsCount = GetNumberDigits(number);
        QCOMPARE(digitsCount, 7U);
        number = L"-3432";
        digitsCount = GetNumberDigits(number);
        QCOMPARE(digitsCount, 4U);
        number = L"0";
        digitsCount = GetNumberDigits(number);
        QCOMPARE(digitsCount, 1U);
        number = L"0.0001223";
        digitsCount = GetNumberDigits(number);
        QCOMPARE(digitsCount, 8U);
    }

    static void unitConversionManagerNumberFormattingUtils_GetNumberDigitsWholeNumberPart()
    {
        unsigned int digitsCount = GetNumberDigitsWholeNumberPart(2.10321);
        QCOMPARE(digitsCount, 1U);
        digitsCount = GetNumberDigitsWholeNumberPart(-122.1232);
        QCOMPARE(digitsCount, 3U);
        digitsCount = GetNumberDigitsWholeNumberPart(-3432);
        QCOMPARE(digitsCount, 4U);
        digitsCount = GetNumberDigitsWholeNumberPart(0);
        QCOMPARE(digitsCount, 1U);
        digitsCount = GetNumberDigitsWholeNumberPart(324328412837382);
        QCOMPARE(digitsCount, 15U);
        digitsCount = GetNumberDigitsWholeNumberPart(324328412837382.232213214324234);
        QCOMPARE(digitsCount, 15U);
        digitsCount = GetNumberDigitsWholeNumberPart(0.032);
        QCOMPARE(digitsCount, 1U);
        digitsCount = GetNumberDigitsWholeNumberPart(0.00000000000000000001);
        QCOMPARE(digitsCount, 1U);
    }

    static void unitConversionManagerNumberFormattingUtils_RoundSignificantDigits()
    {
        std::wstring result = RoundSignificantDigits(12.342343242, 3);
        QCOMPARE(text(result), text(L"12.342"));
        result = RoundSignificantDigits(12.3429999, 3);
        QCOMPARE(text(result), text(L"12.343"));
        result = RoundSignificantDigits(12.342500001, 3);
        QCOMPARE(text(result), text(L"12.343"));
        result = RoundSignificantDigits(-2312.1244243346454345, 5);
        QCOMPARE(text(result), text(L"-2312.12442"));
        result = RoundSignificantDigits(0.3423432423, 5);
        QCOMPARE(text(result), text(L"0.34234"));
        result = RoundSignificantDigits(0.3423, 7);
        QCOMPARE(text(result), text(L"0.3423000"));
    }

    static void unitConversionManagerNumberFormattingUtils_ToScientificNumber()
    {
        std::wstring result = ToScientificNumber(3423);
        QCOMPARE(text(result), text(L"3.423000e+03"));
        result = ToScientificNumber(-21);
        QCOMPARE(text(result), text(L"-2.100000e+01"));
        result = ToScientificNumber(0.0232);
        QCOMPARE(text(result), text(L"2.320000e-02"));
        result = ToScientificNumber(-0.00921);
        QCOMPARE(text(result), text(L"-9.210000e-03"));
        result = ToScientificNumber(2343243345677);
        QCOMPARE(text(result), text(L"2.343243e+12"));
        result = ToScientificNumber(-3432474247332942);
        QCOMPARE(text(result), text(L"-3.432474e+15"));
        result = ToScientificNumber(0.000000003432432);
        QCOMPARE(text(result), text(L"3.432432e-09"));
        result = ToScientificNumber(-0.000000003432432);
        QCOMPARE(text(result), text(L"-3.432432e-09"));
    }

    void calculatorManagerTestBinaryOperatorReceived()
    {
        const CalculatorManagerDisplayTester* pCalculatorDisplay = m_calculatorDisplayTester.get();

        QCOMPARE(pCalculatorDisplay->GetBinaryOperatorReceivedCallCount(), 0);

        m_calculatorManager->SetStandardMode();
        ExecuteCommands({ Command::Command1, Command::CommandADD });

        const std::wstring display = pCalculatorDisplay->GetPrimaryDisplay();
        QCOMPARE(text(display), text(L"1"));

        // Verify BinaryOperatorReceived
        QCOMPARE(pCalculatorDisplay->GetBinaryOperatorReceivedCallCount(), 1);
    }

    void calculatorManagerTestBinaryOperatorReceived_Multiple()
    {
        const CalculatorManagerDisplayTester* pCalculatorDisplay = m_calculatorDisplayTester.get();

        QCOMPARE(pCalculatorDisplay->GetBinaryOperatorReceivedCallCount(), 0);

        m_calculatorManager->SetStandardMode();
        ExecuteCommands({ Command::Command1, Command::CommandADD, Command::CommandSUB, Command::CommandMUL });

        const std::wstring display = pCalculatorDisplay->GetPrimaryDisplay();
        QCOMPARE(text(display), text(L"1"));

        // Verify BinaryOperatorReceived
        QCOMPARE(pCalculatorDisplay->GetBinaryOperatorReceivedCallCount(), 3);
    }

    void calculatorManagerTestBinaryOperatorReceived_LongInput()
    {
        const CalculatorManagerDisplayTester* pCalculatorDisplay = m_calculatorDisplayTester.get();

        QCOMPARE(pCalculatorDisplay->GetBinaryOperatorReceivedCallCount(), 0);

        m_calculatorManager->SetStandardMode();
        ExecuteCommands({ Command::Command1,
                          Command::CommandADD,
                          Command::Command2,
                          Command::CommandMUL,
                          Command::Command1,
                          Command::Command0,
                          Command::CommandSUB,
                          Command::Command5,
                          Command::CommandDIV,
                          Command::Command5,
                          Command::CommandEQU });

        const std::wstring display = pCalculatorDisplay->GetPrimaryDisplay();
        QCOMPARE(text(display), text(L"5"));

        // Verify BinaryOperatorReceived
        QCOMPARE(pCalculatorDisplay->GetBinaryOperatorReceivedCallCount(), 4);
    }

    void calculatorManagerTestStandardOrderOfOperations()
    {
        const std::vector<Command> commands1 = { Command::Command1, Command::CommandREC, Command::CommandNULL };
        m_testDriver.test(L"1", L"1/(1)", commands1);

        const std::vector<Command> commands2 = { Command::Command4, Command::CommandSQRT, Command::CommandNULL };
        m_testDriver.test(L"2", L"\x221A(4)", commands2);

        const std::vector<Command> commands3 = { Command::Command1, Command::CommandADD, Command::Command4, Command::CommandSQRT, Command::CommandNULL };
        m_testDriver.test(L"2", L"1 + \x221A(4)", commands3);

        const std::vector<Command> commands4 = { Command::Command1, Command::CommandADD, Command::Command4, Command::CommandSQRT, Command::CommandSUB, Command::CommandNULL };
        m_testDriver.test(L"3", L"3 - ", commands4);

        const std::vector<Command> commands5 = { Command::Command2, Command::CommandMUL, Command::Command4, Command::CommandREC, Command::CommandNULL };
        m_testDriver.test(L"0.25", L"2 \x00D7 1/(4)", commands5);

        const std::vector<Command> commands6 = { Command::Command5, Command::CommandDIV, Command::Command6, Command::CommandPERCENT, Command::CommandNULL };
        m_testDriver.test(L"0.06", L"5 \x00F7 0.06", commands6);

        const std::vector<Command> commands7 = { Command::Command4, Command::CommandSQRT, Command::CommandSUB, Command::CommandNULL };
        m_testDriver.test(L"2", L"\x221A(4) - ", commands7);

        const std::vector<Command> commands8 = { Command::Command7, Command::CommandSQR, Command::CommandDIV, Command::CommandNULL };
        m_testDriver.test(L"49", L"sqr(7) \x00F7 ", commands8);

        const std::vector<Command> commands9 = { Command::Command8, Command::CommandSQR, Command::CommandSQRT, Command::CommandNULL };
        m_testDriver.test(L"8", L"\x221A(sqr(8))", commands9);

        const std::vector<Command> commands10 = { Command::Command1, Command::Command0, Command::CommandADD, Command::Command2, Command::CommandSUB, Command::CommandNULL };
        m_testDriver.test(L"12", L"12 - ", commands10);

        const std::vector<Command> commands11 = { Command::Command3, Command::CommandMUL, Command::Command4, Command::CommandDIV, Command::CommandNULL };
        m_testDriver.test(L"12", L"12 \x00F7 ", commands11);

        const std::vector<Command> commands12 = { Command::Command6, Command::CommandDIV, Command::Command3, Command::CommandSUB, Command::CommandADD, Command::CommandNULL };
        m_testDriver.test(L"2", L"2 + ", commands12);

        const std::vector<Command> commands13 = { Command::Command7, Command::CommandSUB, Command::Command4, Command::CommandDIV, Command::CommandMUL, Command::CommandNULL };
        m_testDriver.test(L"3", L"3 \x00D7 ", commands13);

        const std::vector<Command> commands14 = { Command::Command8, Command::CommandMUL, Command::Command2, Command::CommandADD, Command::CommandSQRT, Command::CommandNULL };
        m_testDriver.test(L"4", L"16 + \x221A(16)", commands14);

        const std::vector<Command> commands15 = { Command::Command9, Command::CommandADD, Command::Command0, Command::CommandMUL, Command::CommandSIGN, Command::CommandNULL };
        m_testDriver.test(L"-9", L"9 \x00D7 negate(9)", commands15);

        const std::vector<Command> commands16 = { Command::Command9, Command::CommandSIGN, Command::Command0, Command::CommandADD, Command::CommandMUL, Command::CommandNULL };
        m_testDriver.test(L"-90", L"-90 \x00D7 ", commands16);

        const std::vector<Command> commands17 = { Command::Command1, Command::CommandADD, Command::Command2, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"3", L"1 + 2=", commands17);

        const std::vector<Command> commands18 = { Command::Command2, Command::Command0,   Command::CommandMUL, Command::Command0,
                                 Command::Command2, Command::CommandEQU, Command::CommandNULL };
        m_testDriver.test(L"40", L"20 \x00D7 2=", commands18);

        const std::vector<Command> commands19 = { Command::Command1, Command::CommandADD, Command::Command2, Command::CommandADD, Command::CommandBACK, Command::CommandNULL };
        m_testDriver.test(L"3", L"3 + ", commands19);

        const std::vector<Command> commands20 = { Command::Command1, Command::CommandADD, Command::Command2, Command::CommandADD, Command::CommandCLEAR, Command::CommandNULL };
        m_testDriver.test(L"0", L"", commands20);

        const std::vector<Command> commands21 = { Command::Command1, Command::CommandADD, Command::Command2, Command::CommandADD, Command::CommandCENTR, Command::CommandNULL };
        m_testDriver.test(L"0", L"3 + ", commands21);

        const std::vector<Command> commands22 = { Command::Command1, Command::CommandADD, Command::Command2, Command::CommandCLEAR, Command::CommandNULL };
        m_testDriver.test(L"0", L"", commands22);

        const std::vector<Command> commands23 = { Command::Command1, Command::CommandADD, Command::Command2, Command::CommandCENTR, Command::CommandNULL };
        m_testDriver.test(L"0", L"1 + ", commands23);

        const std::vector<Command> commands24 = { Command::Command1, Command::CommandMUL, Command::Command2, Command::CommandMUL, Command::Command3,   Command::CommandMUL,
                                 Command::Command4, Command::CommandMUL, Command::Command5, Command::CommandMUL, Command::CommandNULL };
        m_testDriver.test(L"120", L"120 \x00D7 ", commands24);
    }

};

QTEST_GUILESS_MAIN(TestEngineCalculatormanager)
#include "test_engine_calculatormanager.moc"
