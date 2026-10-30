// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// Ported from microsoft/calculator commit 79e1ac5, src/CalculatorUnitTests/CalcEngineTests.cpp, for Chrono Calc.

#include "engine_resource_provider.h"

#include "CalcManager/CalculatorHistory.h"
#include "CalcManager/CalculatorResource.h"
#include "Header Files/CalcEngine.h"
#include "Header Files/EngineStrings.h"

#include <QObject>
#include <QTest>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// tests/engine/test_engine_calcengine.cpp 0.0.3

// --- Version history ----------------------------------------------------
// v0.0.3: Initial: Microsoft's CalcEngineTests.cpp ported to Qt Test, every check and expected value unchanged

using CalculationManager::CalculatorHistory;
using CalculationManager::IResourceProvider;
using chrono_calc::engine_tests::EngineResourceProvider;

namespace {

constexpr size_t MAX_HISTORY_SIZE = 20;

}  // namespace

// Not in an anonymous namespace: CCalcEngine names CalculatorEngineTests::CalcEngineTests as its friend (the class of
// Microsoft's original test). That name is used here only to reach the engine's private functions the tests check.
namespace CalculatorEngineTests {

class CalcEngineTests
{
public:
    static int IsNumberInvalid(const CCalcEngine& engine, const std::wstring& numberString, int iMaxExp, int iMaxMantissa, uint32_t radix)
    {
        return engine.IsNumberInvalid(numberString, iMaxExp, iMaxMantissa, radix);
    }

    static std::vector<uint32_t> DigitGroupingStringToGroupingVector(std::wstring_view groupingString)
    {
        return CCalcEngine::DigitGroupingStringToGroupingVector(groupingString);
    }

    static std::wstring GroupDigits(
        CCalcEngine& engine, std::wstring_view delimiter, std::vector<uint32_t> const& grouping, std::wstring_view displayString, bool isNumNegative)
    {
        return engine.GroupDigits(delimiter, grouping, displayString, isNumNegative);
    }
};

}  // namespace CalculatorEngineTests

using CalculatorEngineTests::CalcEngineTests;

class TestEngineCalcengine : public QObject
{
    Q_OBJECT

    std::unique_ptr<CCalcEngine> m_calcEngine;
    std::shared_ptr<IResourceProvider> m_resourceProvider;
    std::shared_ptr<CalculatorHistory> m_history;

private slots:
    void init();
    void cleanup();

    void testGroupDigitsPerRadix();
    void testIsNumberInvalid();
    static void testDigitGroupingStringToGroupingVector();
    void testGroupDigits();
};

void TestEngineCalcengine::init()
{
    m_resourceProvider = std::make_shared<EngineResourceProvider>();
    m_history = std::make_shared<CalculatorHistory>(MAX_HISTORY_SIZE);
    CCalcEngine::InitialOneTimeOnlySetup(*(m_resourceProvider.get()));
    m_calcEngine = std::make_unique<CCalcEngine>(
        false /* Respect Order of Operations */, false /* Set to Integer Mode */, m_resourceProvider.get(), nullptr, m_history);
}

void TestEngineCalcengine::cleanup()
{
    m_resourceProvider = nullptr;
    m_history = nullptr;
    m_calcEngine = nullptr;
}

void TestEngineCalcengine::testGroupDigitsPerRadix()
{
    // Empty/Error cases
    QVERIFY2(m_calcEngine->GroupDigitsPerRadix(L"", 10).empty(), "Verify grouping empty string returns empty string.");
    QCOMPARE(m_calcEngine->GroupDigitsPerRadix(L"12345678", 9), std::wstring(L"12345678")); // Verify grouping on invalid base returns original string

    // Octal
    QCOMPARE(m_calcEngine->GroupDigitsPerRadix(L"1234567", 8), std::wstring(L"1 234 567")); // Verify grouping in octal.
    QCOMPARE(m_calcEngine->GroupDigitsPerRadix(L"123", 8), std::wstring(L"123")); // Verify minimum grouping in octal.

    // Binary/Hexadecimal
    QCOMPARE(m_calcEngine->GroupDigitsPerRadix(L"1234567890", 2), std::wstring(L"12 3456 7890")); // Verify grouping in binary.
    QCOMPARE(m_calcEngine->GroupDigitsPerRadix(L"1234", 2), std::wstring(L"1234")); // Verify minimum grouping in binary.
    QCOMPARE(m_calcEngine->GroupDigitsPerRadix(L"1234567890", 16), std::wstring(L"12 3456 7890")); // Verify grouping in hexadecimal.
    QCOMPARE(m_calcEngine->GroupDigitsPerRadix(L"1234", 16), std::wstring(L"1234")); // Verify minimum grouping in hexadecimal.

    // Decimal
    QCOMPARE(m_calcEngine->GroupDigitsPerRadix(L"1234567890", 10), std::wstring(L"1,234,567,890")); // Verify grouping in base10.
    QCOMPARE(m_calcEngine->GroupDigitsPerRadix(L"1234567.89", 10), std::wstring(L"1,234,567.89")); // Verify grouping in base10 with decimal.
    QCOMPARE(m_calcEngine->GroupDigitsPerRadix(L"1234567e89", 10), std::wstring(L"1,234,567e89")); // Verify grouping in base10 with exponent.
    QCOMPARE(m_calcEngine->GroupDigitsPerRadix(L"1234567.89e5", 10), std::wstring(L"1,234,567.89e5")); // Verify grouping in base10 with decimal and exponent.
    QCOMPARE(m_calcEngine->GroupDigitsPerRadix(L"-123456789", 10), std::wstring(L"-123,456,789")); // Verify grouping in base10 with negative.
}

void TestEngineCalcengine::testIsNumberInvalid()
{
    // Binary Number Checks
    const std::vector<std::wstring> validBinStrs{L"0", L"1", L"0011", L"1100"};
    const std::vector<std::wstring> invalidBinStrs{L"2", L"A", L"0.1"};
    for (std::wstring const& str : validBinStrs)
    {
        QCOMPARE(CalcEngineTests::IsNumberInvalid(*m_calcEngine, str, 0, 0, 2 /* Binary */), 0);
    }
    for (std::wstring const& str : invalidBinStrs)
    {
        QCOMPARE(CalcEngineTests::IsNumberInvalid(*m_calcEngine, str, 0, 0, 2 /* Binary */), IDS_ERR_UNK_CH);
    }

    // Octal Number Checks
    const std::vector<std::wstring> validOctStrs{L"0", L"7", L"01234567", L"76543210"};
    const std::vector<std::wstring> invalidOctStrs{L"8", L"A", L"0.7"};
    for (std::wstring const& str : validOctStrs)
    {
        QCOMPARE(CalcEngineTests::IsNumberInvalid(*m_calcEngine, str, 0, 0, 8 /* Octal */), 0);
    }
    for (std::wstring const& str : invalidOctStrs)
    {
        QCOMPARE(CalcEngineTests::IsNumberInvalid(*m_calcEngine, str, 0, 0, 8 /* Octal */), IDS_ERR_UNK_CH);
    }

    // Hexadecimal Number Checks
    const std::vector<std::wstring> validHexStrs{L"0", L"F", L"0123456789ABCDEF", L"FEDCBA9876543210"};
    const std::vector<std::wstring> invalidHexStrs{L"G", L"abcdef", L"x", L"0.1"};
    for (std::wstring const& str : validHexStrs)
    {
        QCOMPARE(CalcEngineTests::IsNumberInvalid(*m_calcEngine, str, 0, 0, 16 /* HEx */), 0);
    }
    for (std::wstring const& str : invalidHexStrs)
    {
        QCOMPARE(CalcEngineTests::IsNumberInvalid(*m_calcEngine, str, 0, 0, 16 /* Hex */), IDS_ERR_UNK_CH);
    }

    // Decimal Number Checks

    // Special case errors: long exponent, long mantissa
    const std::wstring longExp(L"1e12345");
    QCOMPARE(CalcEngineTests::IsNumberInvalid(*m_calcEngine, longExp, 5 /* Max exp length */, 100, 10 /* Decimal */), 0);
    QCOMPARE(CalcEngineTests::IsNumberInvalid(*m_calcEngine, longExp, 4 /* Max exp length */, 100, 10 /* Decimal */), IDS_ERR_INPUT_OVERFLOW);
    // Mantissa length is sum of:
    //  - digits before decimal separator, minus leading zeroes
    //  - digits after decimal separator, including trailing zeroes
    // Each of these mantissa values should calculate as a length of 5
    const std::vector<std::wstring> longMantStrs{L"10000", L"10.000", L"0000012345", L"123.45", L"0.00123", L"0.12345", L"-123.45e678"};
    for (std::wstring const& str : longMantStrs)
    {
        QCOMPARE(CalcEngineTests::IsNumberInvalid(*m_calcEngine, str, 100, 5 /* Max mantissa length */, 10 /* Decimal */), 0);
    }
    for (std::wstring const& str : longMantStrs)
    {
        QCOMPARE(CalcEngineTests::IsNumberInvalid(*m_calcEngine, str, 100, 4 /* Max mantissa length */, 10 /* Decimal */), IDS_ERR_INPUT_OVERFLOW);
    }

    // Regex matching (descriptions taken from CalcUtils.cpp)
    // Use 100 for exp/mantissa length as they are tested above
    const std::vector<std::wstring> validDecStrs{// Start with an optional + or -
                                                 L"+1",
                                                 L"-1",
                                                 L"1",
                                                 // Followed by zero or more digits
                                                 L"-",
                                                 L"",
                                                 L"1234567890",
                                                 // Followed by an optional decimal point
                                                 L"1.0",
                                                 L"-.",
                                                 L"1.",
                                                 // Followed by zero or more digits
                                                 L"0.0",
                                                 L"0.123456",
                                                 // Followed by an optional exponent ('e')
                                                 L"1e",
                                                 L"1.e",
                                                 L"-e",
                                                 // If there's an exponent, its optionally followed by + or -
                                                 // and followed by zero or more digits
                                                 L"1e+12345",
                                                 L"1e-12345",
                                                 L"1e123",
                                                 // All together
                                                 L"-123.456e+789"};
    const std::vector<std::wstring> invalidDecStrs{L"x123", L"123-", L"1e1.2", L"1-e2"};
    for (std::wstring const& str : validDecStrs)
    {
        QCOMPARE(CalcEngineTests::IsNumberInvalid(*m_calcEngine, str, 100, 100, 10 /* Dec */), 0);
    }
    for (std::wstring const& str : invalidDecStrs)
    {
        QCOMPARE(CalcEngineTests::IsNumberInvalid(*m_calcEngine, str, 100, 100, 10 /* Dec */), IDS_ERR_UNK_CH);
    }
}

void TestEngineCalcengine::testDigitGroupingStringToGroupingVector()
{
    std::vector<uint32_t> groupingVector{};
    QCOMPARE(CalcEngineTests::DigitGroupingStringToGroupingVector(L""), groupingVector); // Verify empty grouping

    groupingVector = {1};
    QCOMPARE(CalcEngineTests::DigitGroupingStringToGroupingVector(L"1"), groupingVector); // Verify simple grouping

    groupingVector = {3, 0};
    QCOMPARE(CalcEngineTests::DigitGroupingStringToGroupingVector(L"3;0"), groupingVector); // Verify standard grouping

    groupingVector = {3, 0, 0};
    QCOMPARE(CalcEngineTests::DigitGroupingStringToGroupingVector(L"3;0;0"), groupingVector); // Verify expanded non-repeating grouping

    groupingVector = {5, 3, 2, 4, 6};
    QCOMPARE(CalcEngineTests::DigitGroupingStringToGroupingVector(L"5;3;2;4;6"), groupingVector); // Verify long grouping

    groupingVector = {15, 15, 15, 0};
    QCOMPARE(CalcEngineTests::DigitGroupingStringToGroupingVector(L"15;15;15;0"), groupingVector); // Verify large grouping

    groupingVector = {4, 7, 0};
    QCOMPARE(CalcEngineTests::DigitGroupingStringToGroupingVector(L"4;16;7;25;0"), groupingVector); // Verify we ignore oversize grouping

    groupingVector = {3, 0};
    constexpr std::wstring_view nonRepeatingGrouping = L"3;0;0";
    constexpr std::wstring_view repeatingGrouping = nonRepeatingGrouping.substr(0, 3);
    QCOMPARE(CalcEngineTests::DigitGroupingStringToGroupingVector(repeatingGrouping), groupingVector); // Verify we don't go past the end of wstring_view range
}

void TestEngineCalcengine::testGroupDigits()
{
    std::wstring result{L"1234567"};
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L"", {3, 0}, L"1234567", false), result); // Verify handling of empty delimiter.
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {}, L"1234567", false), result); // Verify handling of empty grouping.

    result = L"1,234,567";
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {3, 0}, L"1234567", false), result); // Verify standard digit grouping.

    result = L"1 234 567";
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L" ", {3, 0}, L"1234567", false), result); // Verify delimiter change.

    result = L"1|||234|||567";
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L"|||", {3, 0}, L"1234567", false), result); // Verify long delimiter.

    result = L"12,345e67";
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {3, 0}, L"12345e67", false), result); // Verify respect of exponent.

    result = L"12,345.67";
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {3, 0}, L"12345.67", false), result); // Verify respect of decimal.

    result = L"1,234.56e7";
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {3, 0}, L"1234.56e7", false), result); // Verify respect of exponent and decimal.

    result = L"-1,234,567";
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {3, 0}, L"-1234567", true), result); // Verify negative number grouping.

    // Test various groupings
    result = L"1234567890123456";
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {0, 0}, L"1234567890123456", false), result); // Verify no grouping.

    result = L"1234567890123,456";
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {3}, L"1234567890123456", false), result); // Verify non-repeating grouping.
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {3, 0, 0}, L"1234567890123456", false), result); // Verify expanded form non-repeating grouping.

    result = L"12,34,56,78,901,23456";
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {5, 3, 2, 0}, L"1234567890123456", false), result); // Verify multigroup with repeating grouping.

    result = L"1234,5678,9012,3456";
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {4, 0}, L"1234567890123456", false), result); // Verify repeating non-standard grouping.

    result = L"123456,78,901,23456";
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {5, 3, 2}, L"1234567890123456", false), result); // Verify multigroup non-repeating grouping.
    QCOMPARE(CalcEngineTests::GroupDigits(*m_calcEngine, L",", {5, 3, 2, 0, 0}, L"1234567890123456", false), result); // Verify expanded form multigroup non-repeating grouping.
}


QTEST_GUILESS_MAIN(TestEngineCalcengine)
#include "test_engine_calcengine.moc"
