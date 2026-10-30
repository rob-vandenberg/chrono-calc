#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

// --- Version ------------------------------------------------------------
// src/ui/keyboardlayouts.h 0.3.10

// --- Version history ----------------------------------------------------
// v0.3.10: Initial: which keyboard layout a key comes from, Latin or not

namespace chrono_calc {

// Whether a character is a letter or mark of a script other than Latin (Cyrillic, Greek, Thai, Arabic)
bool isOtherScriptLetter(char32_t character);

// The character of an XKB key symbol (Linux), 0 for none
char32_t characterOfKeySymbol(std::uint32_t keySymbol);

// A key as XKB gives it: its key code (the PC scan code plus 8) and the symbol it gives in the layout in use
struct XkbKey
{
    std::uint32_t code = 0;
    std::uint32_t symbol = 0;
};

// The keyboard layouts in use on Linux (XKB), for a key that types a sign on a letter's place: Windows 7 reads such a
// key as the letter of its place when the layout is not Latin (the Greek Q key types ; and is Q), and as its sign when
// the layout is Latin (the AZERTY key on the place of the US M types , and is a comma). Qt does not say which layout a
// key comes from, so the layouts are read from the system: on Wayland (also under XWayland) the keyboard description
// of the desktop, on X11 the names the desktop set (_XKB_RULES_NAMES), elsewhere XKB's defaults (XKB_DEFAULT_LAYOUT,
// as in the tests).
class KeyboardLayouts
{
public:
    enum class Source : std::uint8_t
    {
        X11,
        Wayland,
        Defaults,
    };

    explicit KeyboardLayouts(Source source);
    // Layouts given by name, for example "us,gr" with no variants
    KeyboardLayouts(std::string layouts, std::string variants);
    ~KeyboardLayouts();
    KeyboardLayouts(const KeyboardLayouts&) = delete;
    KeyboardLayouts& operator=(const KeyboardLayouts&) = delete;
    KeyboardLayouts(KeyboardLayouts&&) = delete;
    KeyboardLayouts& operator=(KeyboardLayouts&&) = delete;

    // The layouts of this computer, by the platform Qt runs on; read when first needed
    static KeyboardLayouts& system();

    // Whether the key, giving its symbol, comes from a layout whose letters are not Latin:
    // true only when every layout in use that gives the symbol on that key is such a layout. When none gives it, the
    // layouts are read again (one may have been added; at most once every 2 seconds), and then it is false.
    bool isFromNonLatinLayout(XkbKey key);

private:
    struct Keymap;

    bool load();
    bool loadIfDue();
    std::optional<bool> lookUp(XkbKey key);
    bool isNonLatinLayout(std::uint32_t layout);

    Source m_source;
    std::string m_layouts;
    std::string m_variants;
    std::unique_ptr<Keymap> m_keymap;
    std::optional<std::chrono::steady_clock::time_point> m_lastLoad;
};

}  // namespace chrono_calc
