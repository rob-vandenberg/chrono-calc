// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// Ported from microsoft/calculator commit 79e1ac5, src/CalculatorUnitTests/CalcInputTest.cpp, for Chrono Calc.

#include "Header Files/CalcInput.h"

#include <QDebug>
#include <QObject>
#include <QString>
#include <QTest>

#include <cstddef>
#include <string>

// --- Version ------------------------------------------------------------
// tests/engine/test_engine_calcinput.cpp 0.0.3

// --- Version history ----------------------------------------------------
// v0.0.3: Initial: Microsoft's CalcInputTest.cpp ported to Qt Test, every check and expected value unchanged

class TestEngineCalcinput : public QObject
{
    Q_OBJECT

    CalcEngine::CalcInput m_calcInput;

private slots:
    void init();
    void cleanup();

    void clear();
    void tryToggleSignZero();
    void tryToggleSignExponent();
    void tryToggleSignBase();
    void tryToggleSignBaseIntegerMode();
    void tryToggleSignRollover();
    void tryAddDigitLeadingZeroes();
    void tryAddDigitMaxCount();
    void tryAddDigitValues();
    void tryAddDigitRolloverBaseCheck();
    void tryAddDigitRolloverOctalByte();
    void tryAddDigitRolloverOctalWord();
    void tryAddDigitRolloverOctalDword();
    void tryAddDigitRolloverOctalQword();
    void tryAddDigitRolloverDecimal();
    void tryAddDecimalPtEmpty();
    void tryAddDecimalPointTwice();
    void tryAddDecimalPointExponent();
    void tryBeginExponentNoExponent();
    void tryBeginExponentWithExponent();
    void backspaceZero();
    void backspaceSingleChar();
    void backspaceMultiChar();
    void backspaceDecimal();
    void backspaceMultiCharDecimal();
    void backspaceZeroDecimalWithoutPrefixZeros();
    void setDecimalSymbol();
    void toStringEmpty();
    void toStringNegative();
    void toStringExponentBase10();
    void toStringExponentBase8();
    void toStringExponentNegative();
    void toStringExponentPositive();
    void toStringInteger();
    void toStringBaseTooLong();
    void toStringExponentTooLong();
    void toRational();
};

void TestEngineCalcinput::init()
{
    m_calcInput = CalcEngine::CalcInput(L'.');
}

void TestEngineCalcinput::cleanup()
{
    m_calcInput.Clear();
    m_calcInput.SetDecimalSymbol(L'.');
}

void TestEngineCalcinput::clear()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryToggleSign(false, L"999");
    m_calcInput.TryAddDecimalPt();
    m_calcInput.TryAddDigit(2, 10, false, L"999", 64, 32);
    m_calcInput.TryBeginExponent();
    m_calcInput.TryAddDigit(3, 10, false, L"999", 64, 32);

    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"-1.2e+3")); // Verify input is correct.

    m_calcInput.Clear();

    qInfo().noquote() << QString::fromStdWString(m_calcInput.ToString(10));
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"0")); // Verify input is 0 after clear.
}

void TestEngineCalcinput::tryToggleSignZero()
{
    QVERIFY2(m_calcInput.TryToggleSign(false, L"999"), "Verify toggling 0 succeeds.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"0")); // Verify toggling 0 does not create -0.
}

void TestEngineCalcinput::tryToggleSignExponent()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryBeginExponent();
    m_calcInput.TryAddDigit(2, 10, false, L"999", 64, 32);
    QVERIFY2(m_calcInput.TryToggleSign(false, L"999"), "Verify toggling exponent sign succeeds.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1.e-2")); // Verify toggling exponent sign does not toggle base sign.
    QVERIFY2(m_calcInput.TryToggleSign(false, L"999"), "Verify toggling exponent sign succeeds.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1.e+2")); // Verify toggling negative exponent sign does not toggle base sign.
}

void TestEngineCalcinput::tryToggleSignBase()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    QVERIFY2(m_calcInput.TryToggleSign(false, L"999"), "Verify toggling base sign succeeds.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"-1")); // Verify toggling base sign creates negative base.
    QVERIFY2(m_calcInput.TryToggleSign(false, L"999"), "Verify toggling base sign succeeds.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1")); // Verify toggling negative base sign creates positive base.
}

void TestEngineCalcinput::tryToggleSignBaseIntegerMode()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    QVERIFY2(m_calcInput.TryToggleSign(true, L"999"), "Verify toggling base sign in integer mode succeeds.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"-1")); // Verify toggling base sign creates negative base.
}

void TestEngineCalcinput::tryToggleSignRollover()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryAddDigit(2, 10, false, L"999", 64, 32);
    QVERIFY2(m_calcInput.TryToggleSign(true, L"127"), "Verify toggling base sign in integer mode succeeds.");
    m_calcInput.TryAddDigit(8, 10, false, L"999", 64, 32);
    QVERIFY2(!m_calcInput.TryToggleSign(true, L"127"), "Verify toggling base sign in integer mode fails on rollover.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"-128")); // Verify toggling base sign on rollover does not change value.
}

void TestEngineCalcinput::tryAddDigitLeadingZeroes()
{
    QVERIFY2(m_calcInput.TryAddDigit(0, 10, false, L"999", 64, 32), "Verify TryAddDigit succeeds.");
    QVERIFY2(m_calcInput.TryAddDigit(0, 10, false, L"999", 64, 32), "Verify TryAddDigit succeeds.");
    QVERIFY2(m_calcInput.TryAddDigit(0, 10, false, L"999", 64, 32), "Verify TryAddDigit succeeds.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"0")); // Verify leading zeros are ignored.
}

void TestEngineCalcinput::tryAddDigitMaxCount()
{
    QVERIFY2(m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32), "Verify TryAddDigit for base with length < maxDigits succeeds.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1")); // Verify adding digit for base with length < maxDigits succeeded.
    QVERIFY2(!m_calcInput.TryAddDigit(2, 10, false, L"999", 64, 1), "Verify TryAddDigit for base with length > maxDigits fails.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1")); // Verify digit for base was not added.
    m_calcInput.TryBeginExponent();
    QVERIFY2(m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32), "Verify TryAddDigit for exponent with length < maxDigits succeeds.");
    QVERIFY2(m_calcInput.TryAddDigit(2, 10, false, L"999", 64, 32), "Verify TryAddDigit for exponent with length < maxDigits succeeds.");
    QVERIFY2(m_calcInput.TryAddDigit(3, 10, false, L"999", 64, 32), "Verify TryAddDigit for exponent with length < maxDigits succeeds.");
    QVERIFY2(m_calcInput.TryAddDigit(4, 10, false, L"999", 64, 32), "Verify TryAddDigit for exponent with length < maxDigits succeeds.");
    QVERIFY2(!m_calcInput.TryAddDigit(5, 10, false, L"999", 64, 32), "Verify TryAddDigit for exponent with length > maxDigits fails.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1.e+1234")); // Verify adding digits for exponent with length < maxDigits succeeded.

    m_calcInput.Clear();
    m_calcInput.TryAddDecimalPt();
    QVERIFY2(m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 1), "Verify decimal point and leading zero does not count toward maxDigits.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"0.1")); // Verify input value checking dec pt and leading zero impact on maxDigits.
}

void TestEngineCalcinput::tryAddDigitValues()
{
    // Use an arbitrary value > 16 to test that input accepts digits > hexadecimal 0xF.
    // TryAddDigit does not validate whether the digit fits within the current radix.
    for (unsigned int i = 0; i < 25; i++)
    {
        QVERIFY2(m_calcInput.TryAddDigit(i, 10, false, L"999", 64, 32), ("Verify TryAddDigit succeeds for " + std::to_string(i)).c_str());
        m_calcInput.Clear();
    }
}

void TestEngineCalcinput::tryAddDigitRolloverBaseCheck()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    QVERIFY2(!m_calcInput.TryAddDigit(2, 16, true, L"999", 64, 1), "Verify TryAddDigit rollover fails for bases other than 8,10.");
    QVERIFY2(!m_calcInput.TryAddDigit(1, 2, true, L"999", 64, 1), "Verify TryAddDigit rollover fails for bases other than 8,10.");
}

void TestEngineCalcinput::tryAddDigitRolloverOctalByte()
{
    m_calcInput.TryAddDigit(1, 8, true, L"777", 64, 32);
    QVERIFY2(m_calcInput.TryAddDigit(2, 8, true, L"377", 8, 1), "Verify we can add an extra digit in OctalByte if first digit <= 3.");

    m_calcInput.Clear();
    m_calcInput.TryAddDigit(4, 8, true, L"777", 64, 32);
    QVERIFY2(!m_calcInput.TryAddDigit(2, 8, true, L"377", 8, 1), "Verify we cannot add an extra digit in OctalByte if first digit > 3.");
}

void TestEngineCalcinput::tryAddDigitRolloverOctalWord()
{
    m_calcInput.TryAddDigit(1, 8, true, L"777", 64, 32);
    QVERIFY2(m_calcInput.TryAddDigit(2, 8, true, L"377", 16, 1), "Verify we can add an extra digit in OctalByte if first digit == 1.");

    m_calcInput.Clear();
    m_calcInput.TryAddDigit(2, 8, true, L"777", 64, 32);
    QVERIFY2(!m_calcInput.TryAddDigit(2, 8, true, L"377", 16, 1), "Verify we cannot add an extra digit in OctalByte if first digit > 1.");
}

void TestEngineCalcinput::tryAddDigitRolloverOctalDword()
{
    m_calcInput.TryAddDigit(1, 8, true, L"777", 64, 32);
    QVERIFY2(m_calcInput.TryAddDigit(2, 8, true, L"377", 32, 1), "Verify we can add an extra digit in OctalByte if first digit <= 3.");

    m_calcInput.Clear();
    m_calcInput.TryAddDigit(4, 8, true, L"777", 64, 32);
    QVERIFY2(!m_calcInput.TryAddDigit(2, 8, true, L"377", 32, 1), "Verify we cannot add an extra digit in OctalByte if first digit > 3.");
}

void TestEngineCalcinput::tryAddDigitRolloverOctalQword()
{
    m_calcInput.TryAddDigit(1, 8, true, L"777", 64, 32);
    QVERIFY2(m_calcInput.TryAddDigit(2, 8, true, L"377", 64, 1), "Verify we can add an extra digit in OctalByte if first digit == 1.");

    m_calcInput.Clear();
    m_calcInput.TryAddDigit(2, 8, true, L"777", 64, 32);
    QVERIFY2(!m_calcInput.TryAddDigit(2, 8, true, L"377", 64, 1), "Verify we cannot add an extra digit in OctalByte if first digit > 1.");
}

void TestEngineCalcinput::tryAddDigitRolloverDecimal()
{
    m_calcInput.TryAddDigit(1, 10, true, L"127", 64, 32);
    QVERIFY2(!m_calcInput.TryAddDigit(0, 10, true, L"1", 8, 1), "Verify we cannot add a digit if input size matches maxStr size.");
    m_calcInput.TryAddDigit(2, 10, true, L"127", 64, 32);
    QVERIFY2(!m_calcInput.TryAddDigit(2, 10, true, L"110", 8, 2), "Verify we cannot add a digit if n char comparison > 0.");
    QVERIFY2(m_calcInput.TryAddDigit(7, 10, true, L"130", 8, 2), "Verify we can add a digit if n char comparison < 0.");

    m_calcInput.Clear();
    m_calcInput.TryAddDigit(1, 10, true, L"127", 64, 32);
    m_calcInput.TryAddDigit(2, 10, true, L"127", 64, 32);
    QVERIFY2(!m_calcInput.TryAddDigit(8, 10, true, L"127", 8, 2), "Verify we cannot add a digit if digit exceeds max value.");
    QVERIFY2(m_calcInput.TryAddDigit(7, 10, true, L"127", 8, 2), "Verify we can add a digit if digit does not exceed max value.");

    m_calcInput.Backspace();
    m_calcInput.TryToggleSign(true, L"127");
    QVERIFY2(!m_calcInput.TryAddDigit(9, 10, true, L"127", 8, 2), "Negative value: verify we cannot add a digit if digit exceeds max value.");
    QVERIFY2(m_calcInput.TryAddDigit(8, 10, true, L"127", 8, 2), "Negative value: verify we can add a digit if digit does not exceed max value.");
}

void TestEngineCalcinput::tryAddDecimalPtEmpty()
{
    QVERIFY2(!m_calcInput.HasDecimalPt(), "Verify input has no decimal point.");
    QVERIFY2(m_calcInput.TryAddDecimalPt(), "Verify adding decimal to empty input.");
    QVERIFY2(m_calcInput.HasDecimalPt(), "Verify input has decimal point.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"0.")); // Verify decimal on empty input.
}

void TestEngineCalcinput::tryAddDecimalPointTwice()
{
    QVERIFY2(!m_calcInput.HasDecimalPt(), "Verify input has no decimal point.");
    QVERIFY2(m_calcInput.TryAddDecimalPt(), "Verify adding decimal to empty input.");
    QVERIFY2(m_calcInput.HasDecimalPt(), "Verify input has decimal point.");
    QVERIFY2(!m_calcInput.TryAddDecimalPt(), "Verify adding decimal point fails if input has decimal point.");
}

void TestEngineCalcinput::tryAddDecimalPointExponent()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryBeginExponent();
    m_calcInput.TryAddDigit(2, 10, false, L"999", 64, 32);
    QVERIFY2(!m_calcInput.TryAddDecimalPt(), "Verify adding decimal point fails if input has exponent.");
}

void TestEngineCalcinput::tryBeginExponentNoExponent()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    QVERIFY2(m_calcInput.TryBeginExponent(), "Verify adding exponent succeeds on input without exponent.");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1.e+0")); // Verify exponent present.
}

void TestEngineCalcinput::tryBeginExponentWithExponent()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    QVERIFY2(m_calcInput.TryBeginExponent(), "Verify adding exponent succeeds on input without exponent.");
    QVERIFY2(!m_calcInput.TryBeginExponent(), "Verify cannot add exponent if input already has exponent.");
}

void TestEngineCalcinput::backspaceZero()
{
    m_calcInput.Backspace();
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"0")); // Verify backspace on 0 is still 0.
}

void TestEngineCalcinput::backspaceSingleChar()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1")); // Verify input before backspace.
    m_calcInput.Backspace();
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"0")); // Verify input after backspace.
}

void TestEngineCalcinput::backspaceMultiChar()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryAddDigit(2, 10, false, L"999", 64, 32);
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"12")); // Verify input before backspace.
    m_calcInput.Backspace();
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1")); // Verify input after backspace.
}

void TestEngineCalcinput::backspaceDecimal()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryAddDecimalPt();
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1.")); // Verify input before backspace.
    QVERIFY2(m_calcInput.HasDecimalPt(), "Verify input has decimal point.");
    m_calcInput.Backspace();
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1")); // Verify input after backspace.
    QVERIFY2(!m_calcInput.HasDecimalPt(), "Verify decimal point was removed.");
}

void TestEngineCalcinput::backspaceMultiCharDecimal()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryAddDecimalPt();
    m_calcInput.TryAddDigit(2, 10, false, L"999", 64, 32);
    m_calcInput.TryAddDigit(3, 10, false, L"999", 64, 32);
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1.23")); // Verify input before backspace.
    m_calcInput.Backspace();
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1.2")); // Verify input after backspace.
}

// Issue #817: Prefixed multiple zeros
void TestEngineCalcinput::backspaceZeroDecimalWithoutPrefixZeros()
{
    m_calcInput.TryAddDigit(0, 10, false, L"999", 64, 32);
    m_calcInput.TryAddDecimalPt();
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"0.")); // Verify input before backspace.
    m_calcInput.Backspace();
    m_calcInput.TryAddDigit(0, 10, false, L"999", 64, 32);
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"0")); // Verify input after backspace.
}

void TestEngineCalcinput::setDecimalSymbol()
{
    m_calcInput.TryAddDecimalPt();
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"0.")); // Verify default decimal point.
    m_calcInput.SetDecimalSymbol(L',');
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"0,")); // Verify new decimal point.
}

void TestEngineCalcinput::toStringEmpty()
{
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"0")); // Verify ToString of empty value.
}

void TestEngineCalcinput::toStringNegative()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryToggleSign(false, L"999");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"-1")); // Verify ToString of negative value.
}

void TestEngineCalcinput::toStringExponentBase10()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryBeginExponent();
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1.e+0")); // Verify ToString of empty base10 exponent.
}

void TestEngineCalcinput::toStringExponentBase8()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryBeginExponent();
    QCOMPARE(m_calcInput.ToString(8), std::wstring(L"1.^+0")); // Verify ToString of empty base8 exponent.
}

void TestEngineCalcinput::toStringExponentNegative()
{
    m_calcInput.TryAddDigit(1, 8, false, L"999", 64, 32);
    m_calcInput.TryBeginExponent();
    m_calcInput.TryToggleSign(false, L"999");
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1.e-0")); // Verify ToString of empty negative exponent.
}

void TestEngineCalcinput::toStringExponentPositive()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryBeginExponent();
    m_calcInput.TryAddDigit(2, 10, false, L"999", 64, 32);
    m_calcInput.TryAddDigit(3, 10, false, L"999", 64, 32);
    m_calcInput.TryAddDigit(4, 10, false, L"999", 64, 32);
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1.e+234")); // Verify ToString of exponent with value.
}

void TestEngineCalcinput::toStringInteger()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"1")); // Verify ToString of integer value hides decimal.
}

void TestEngineCalcinput::toStringBaseTooLong()
{
    std::wstring maxStr{};
    for (size_t i = 0; i < MAX_STRLEN + 1; i++)
    {
        maxStr += L'1';
        m_calcInput.TryAddDigit(1, 10, false, maxStr, 64, 100);
    }
    auto result = m_calcInput.ToString(10);
    QVERIFY2(result.empty(), "Verify ToString of base value that is too large yields empty string.");
}

void TestEngineCalcinput::toStringExponentTooLong()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryBeginExponent();
    std::wstring maxStr{ L"11" };
    bool exponentCapped = false;
    for (size_t i = 0; i < MAX_STRLEN + 1; i++)
    {
        maxStr += L'1';
        if (!m_calcInput.TryAddDigit(1, 10, false, maxStr, 64, MAX_STRLEN + 25))
        {
            exponentCapped = true;
        }
    }
    auto result = m_calcInput.ToString(10);

    // TryAddDigit caps the exponent length to C_EXP_MAX_DIGITS = 4, so ToString() succeeds.
    // If that cap is removed, ToString() should return an empty string.
    if (exponentCapped)
    {
        QCOMPARE(result, std::wstring(L"1.e+1111")); // Verify ToString succeeds; exponent length is capped at C_EXP_MAX_DIGITS.
    }
    else
    {
        QVERIFY2(result.empty(), "Verify ToString of exponent value that is too large yields empty string.");
    }
}

void TestEngineCalcinput::toRational()
{
    m_calcInput.TryAddDigit(1, 10, false, L"999", 64, 32);
    m_calcInput.TryAddDigit(2, 10, false, L"999", 64, 32);
    m_calcInput.TryAddDigit(3, 10, false, L"999", 64, 32);
    QCOMPARE(m_calcInput.ToString(10), std::wstring(L"123")); // Verify input before conversion to rational.

    // Microsoft passes false as the precision, which is the precision 0
    auto rat = m_calcInput.ToRational(10, 0);
    QCOMPARE(rat.P().Mantissa().size(), 1); // Verify digit count of rational.
    QCOMPARE(rat.P().Mantissa().front(), 123); // Verify first digit of mantissa.
}


QTEST_GUILESS_MAIN(TestEngineCalcinput)
#include "test_engine_calcinput.moc"
