#include <QObject>
#include <QTest>

#include "ui/keyboardlayouts.h"

#include <cstdint>

// --- Version ------------------------------------------------------------
// tests/test_keyboardlayouts.cpp 0.3.10

// --- Version history ----------------------------------------------------
// v0.3.10: Initial: which layout a key that types a sign comes from, Latin or not

namespace {

// The XKB key codes (PC scan code plus 8) of the keys on the places of the US Q and M, and the symbols ; / , '
constexpr std::uint32_t qKey = 0x10 + 8;
constexpr std::uint32_t mKey = 0x32 + 8;
constexpr std::uint32_t semicolon = 0x3B;
constexpr std::uint32_t slash = 0x2F;
constexpr std::uint32_t comma = 0x2C;
constexpr std::uint32_t apostrophe = 0x27;

}  // namespace

class TestKeyboardLayouts : public QObject
{
    Q_OBJECT

private slots:
    // Only the names each test gives count, not the developer's own XKB settings
    static void initTestCase()
    {
        for (const char* name : {"XKB_DEFAULT_RULES", "XKB_DEFAULT_MODEL", "XKB_DEFAULT_LAYOUT", "XKB_DEFAULT_VARIANT",
                                 "XKB_DEFAULT_OPTIONS"}) {
            qunsetenv(name);
        }
    }

    // Greek and Hebrew are not Latin: their Q key types ; or / and stands for Q (Windows 7, reference part F line 6)
    static void aSignOnALetterPlaceOfGreekOrHebrewIsFromANonLatinLayout()
    {
        chrono_calc::KeyboardLayouts greek("us,gr", ",");
        QVERIFY(greek.isFromNonLatinLayout({.code = qKey, .symbol = semicolon}));
        chrono_calc::KeyboardLayouts hebrew("us,il", ",");
        QVERIFY(hebrew.isFromNonLatinLayout({.code = qKey, .symbol = slash}));
    }

    // AZERTY and Dvorak are Latin: their signs on letter places stay signs; a symbol no layout gives is not looked up
    static void aSignOnALetterPlaceOfALatinLayoutIsNot()
    {
        chrono_calc::KeyboardLayouts azerty("fr", "");
        QVERIFY(!azerty.isFromNonLatinLayout({.code = mKey, .symbol = comma}));
        chrono_calc::KeyboardLayouts dvorak("us", "dvorak");
        QVERIFY(!dvorak.isFromNonLatinLayout({.code = qKey, .symbol = apostrophe}));
        chrono_calc::KeyboardLayouts us("us", "");
        QVERIFY(!us.isFromNonLatinLayout({.code = qKey, .symbol = semicolon}));
    }

    // The characters of key symbols and their scripts
    static void keySymbolsGiveTheirCharacters()
    {
        QCOMPARE(chrono_calc::characterOfKeySymbol(0x6D1), U'я');
        QVERIFY(chrono_calc::isOtherScriptLetter(chrono_calc::characterOfKeySymbol(0x6D1)));
        QVERIFY(chrono_calc::isOtherScriptLetter(U'่'));
        QVERIFY(!chrono_calc::isOtherScriptLetter(U'a'));
        QVERIFY(!chrono_calc::isOtherScriptLetter(U';'));
    }
};

QTEST_GUILESS_MAIN(TestKeyboardLayouts)
#include "test_keyboardlayouts.moc"
