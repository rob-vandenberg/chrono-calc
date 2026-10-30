// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// Ported from microsoft/calculator commit 79e1ac5, src/CalculatorUnitTests/RationalTest.cpp, for Chrono Calc.

#include "Header Files/Number.h"
#include "Header Files/Rational.h"
#include "Header Files/RationalMath.h"
#include "Ratpack/CalcErr.h"
#include "Ratpack/ratpak.h"

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QTest>

#include <cstdint>
#include <limits>
#include <string>

// --- Version ------------------------------------------------------------
// tests/engine/test_engine_rational.cpp 0.0.3

// --- Version history ----------------------------------------------------
// v0.0.3: Initial: Microsoft's RationalTest.cpp ported to Qt Test, every check and expected value unchanged

using CalcEngine::Number;
using CalcEngine::Rational;
using CalcEngine::RATIONAL_PRECISION;
using CalcEngine::RationalMath::Mod;

// Not in an anonymous namespace: this is Qt Test's own way to print a type, a specialization of QTest::toString
namespace QTest {

// How QCOMPARE prints a Rational when a check fails
template <>
char* toString(const Rational& t)
{
    const std::wstring text = t.ToString(10, NumberFormat::Float, RATIONAL_PRECISION);
    return qstrdup(QString::fromStdWString(text).toUtf8().constData());
}

}  // namespace QTest

class TestEngineRational : public QObject
{
    Q_OBJECT

private slots:
    static void initTestCase();

    static void testModuloOperandsNotModified();
    static void testInt32MinIntegerPower();
    static void testModuloInteger();
    static void testModuloZero();
    static void testModuloRational();
    static void testRemainderOperandsNotModified();
    static void testRemainderInteger();
    static void testRemainderZero();
    static void testRemainderRational();
};

void TestEngineRational::initTestCase()
{
    ChangeConstants(10, 128);
}

void TestEngineRational::testModuloOperandsNotModified()
{
    // Verify results but also check that operands are not modified
    const Rational rat25(25);
    const Rational ratminus25(-25);
    const Rational rat4(4);
    const Rational ratminus4(-4);
    Rational res = Mod(rat25, rat4);
    QCOMPARE(res, 1);
    QCOMPARE(rat25, 25);
    QCOMPARE(rat4, 4);
    res = Mod(rat25, ratminus4);
    QCOMPARE(res, -3);
    QCOMPARE(rat25, 25);
    QCOMPARE(ratminus4, -4);
    res = Mod(ratminus25, ratminus4);
    QCOMPARE(res, -1);
    QCOMPARE(ratminus25, -25);
    QCOMPARE(ratminus4, -4);
    res = Mod(ratminus25, rat4);
    QCOMPARE(res, 3);
    QCOMPARE(ratminus25, -25);
    QCOMPARE(rat4, 4);
}

void TestEngineRational::testInt32MinIntegerPower()
{
    PRAT base = Rational(-1).ToPRAT();
    ratpowi32(&base, std::numeric_limits<int32_t>::min(), RATIONAL_PRECISION);
    const Rational result{base};
    destroyrat(base);

    QCOMPARE(result, 1);
}

void TestEngineRational::testModuloInteger()
{
    // Check with integers
    auto res = Mod(Rational(426), Rational(56478));
    QCOMPARE(res, 426);
    res = Mod(Rational(56478), Rational(426));
    QCOMPARE(res, 246);
    res = Mod(Rational(-643), Rational(8756));
    QCOMPARE(res, 8113);
    res = Mod(Rational(643), Rational(-8756));
    QCOMPARE(res, -8113);
    res = Mod(Rational(-643), Rational(-8756));
    QCOMPARE(res, -643);
    res = Mod(Rational(1000), Rational(250));
    QCOMPARE(res, 0);
    res = Mod(Rational(1000), Rational(-250));
    QCOMPARE(res, 0);
}

void TestEngineRational::testModuloZero()
{
    // Test with Zero
    auto res = Mod(Rational(343654332), Rational(0));
    QCOMPARE(res, 343654332);
    res = Mod(Rational(0), Rational(8756));
    QCOMPARE(res, 0);
    res = Mod(Rational(0), Rational(-242));
    QCOMPARE(res, 0);
    res = Mod(Rational(0), Rational(0));
    QCOMPARE(res, 0);
    res = Mod(Rational(Number(1, 0, {23242}), Number(1, 0, {2})), Rational(Number(1, 0, {0}), Number(1, 0, {23})));
    QCOMPARE(res, 11621);
}

void TestEngineRational::testModuloRational()
{
    // Test with rational numbers
    auto res = Mod(Rational(Number(1, 0, {250}), Number(1, 0, {100})), Rational(89));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"2.5"));
    res = Mod(Rational(Number(1, 0, {3330}), Number(1, 0, {1332})), Rational(1));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"0.5"));
    res = Mod(Rational(Number(1, 0, {12250}), Number(1, 0, {100})), Rational(10));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"2.5"));
    res = Mod(Rational(Number(-1, 0, {12250}), Number(1, 0, {100})), Rational(10));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"7.5"));
    res = Mod(Rational(Number(-1, 0, {12250}), Number(1, 0, {100})), Rational(-10));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"-2.5"));
    res = Mod(Rational(Number(1, 0, {12250}), Number(1, 0, {100})), Rational(-10));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"-7.5"));
    res = Mod(Rational(Number(1, 0, {1000}), Number(1, 0, {3})), Rational(1));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"0.33333333"));
    res = Mod(Rational(Number(1, 0, {1000}), Number(1, 0, {3})), Rational(-10));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"-6.6666667"));
    res = Mod(Rational(834345), Rational(Number(1, 0, {103}), Number(1, 0, {100})));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"0.71"));
    res = Mod(Rational(834345), Rational(Number(-1, 0, {103}), Number(1, 0, {100})));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"-0.32"));
}

void TestEngineRational::testRemainderOperandsNotModified()
{
    // Verify results but also check that operands are not modified
    const Rational rat25(25);
    const Rational ratminus25(-25);
    const Rational rat4(4);
    const Rational ratminus4(-4);
    Rational res = rat25 % rat4;
    QCOMPARE(res, 1);
    QCOMPARE(rat25, 25);
    QCOMPARE(rat4, 4);
    res = rat25 % ratminus4;
    QCOMPARE(res, 1);
    QCOMPARE(rat25, 25);
    QCOMPARE(ratminus4, -4);
    res = ratminus25 % ratminus4;
    QCOMPARE(res, -1);
    QCOMPARE(ratminus25, -25);
    QCOMPARE(ratminus4, -4);
    res = ratminus25 % rat4;
    QCOMPARE(res, -1);
    QCOMPARE(ratminus25, -25);
    QCOMPARE(rat4, 4);
}

void TestEngineRational::testRemainderInteger()
{
    // Check with integers
    auto res = Rational(426) % Rational(56478);
    QCOMPARE(res, 426);
    res = Rational(56478) % Rational(426);
    QCOMPARE(res, 246);
    res = Rational(-643) % Rational(8756);
    QCOMPARE(res, -643);
    res = Rational(643) % Rational(-8756);
    QCOMPARE(res, 643);
    res = Rational(-643) % Rational(-8756);
    QCOMPARE(res, -643);
    res = Rational(-124) % Rational(-124);
    QCOMPARE(res, 0);
    res = Rational(24) % Rational(24);
    QCOMPARE(res, 0);
}

void TestEngineRational::testRemainderZero()
{
    // Test with Zero
    auto res = Rational(0) % Rational(3654);
    QCOMPARE(res, 0);
    res = Rational(0) % Rational(-242);
    QCOMPARE(res, 0);
    for (const int32_t number : {343654332, 0, -23423})
    {
        try
        {
            res = Rational(number) % Rational(0);
            QFAIL("Expected the engine error CALC_E_INDEFINITE, but nothing was thrown");
        }
        catch (const uint32_t& t)
        {
            if (t != CALC_E_INDEFINITE)
            {
                QFAIL("Expected the engine error CALC_E_INDEFINITE, but another engine error was thrown");
            }
        }
        catch (...)
        {
            QFAIL("Expected the engine error CALC_E_INDEFINITE, but an exception of another type was thrown");
        }

        try
        {
            res = Rational(Number(1, number, {0}), Number(1, 0, {2})) % Rational(Number(1, 0, {0}), Number(1, 0, {23}));
            QFAIL("Expected the engine error CALC_E_INDEFINITE, but nothing was thrown");
        }
        catch (const uint32_t& t)
        {
            if (t != CALC_E_INDEFINITE)
            {
                QFAIL("Expected the engine error CALC_E_INDEFINITE, but another engine error was thrown");
            }
        }
        catch (...)
        {
            QFAIL("Expected the engine error CALC_E_INDEFINITE, but an exception of another type was thrown");
        }
    }
}

void TestEngineRational::testRemainderRational()
{
    // Test with rational numbers
    auto res = Rational(Number(1, 0, {250}), Number(1, 0, {100})) % Rational(89);
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"2.5"));
    res = Rational(Number(1, 0, {3330}), Number(1, 0, {1332})) % Rational(1);
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"0.5"));
    res = Rational(Number(1, 0, {12250}), Number(1, 0, {100})) % Rational(10);
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"2.5"));
    res = Rational(Number(-1, 0, {12250}), Number(1, 0, {100})) % Rational(10);
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"-2.5"));
    res = Rational(Number(-1, 0, {12250}), Number(1, 0, {100})) % Rational(-10);
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"-2.5"));
    res = Rational(Number(1, 0, {12250}), Number(1, 0, {100})) % Rational(-10);
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"2.5"));
    res = Rational(Number(1, 0, {1000}), Number(1, 0, {3})) % Rational(1);
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"0.33333333"));
    res = Rational(Number(1, 0, {1000}), Number(1, 0, {3})) % Rational(-10);
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"3.3333333"));
    res = Rational(Number(-1, 0, {1000}), Number(1, 0, {3})) % Rational(-10);
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"-3.3333333"));
    res = Rational(834345) % Rational(Number(1, 0, {103}), Number(1, 0, {100}));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"0.71"));
    res = Rational(834345) % Rational(Number(-1, 0, {103}), Number(1, 0, {100}));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"0.71"));
    res = Rational(-834345) % Rational(Number(1, 0, {103}), Number(1, 0, {100}));
    QCOMPARE(res.ToString(10, NumberFormat::Float, 8), std::wstring(L"-0.71"));
}

QTEST_GUILESS_MAIN(TestEngineRational)
#include "test_engine_rational.moc"
